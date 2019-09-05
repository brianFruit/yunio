#!/usr/bin/python
import urllib2
import urlparse
import urllib
import json
import threading
import time
import socket
from BaseHTTPServer import HTTPServer
from BaseHTTPServer import BaseHTTPRequestHandler
import sys
sys.path.insert(0, '/usr/lib/python2.7/bridge/')
from bridgeclient import BridgeClient as bridgeclient

NUMIO = 20
SERVER_IP = None
SERVER_PORT = None
RUN = True


ATmega_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ATmega_socket.connect(('localhost', 6571))

io_state = {}
for io in range(NUMIO):
    io_state[str(io)] = None

def keep_alive():
    while RUN:
        keepalive_endpoint = "http://" + SERVER_IP + ":" + str(SERVER_PORT) + "/keep-alive"
        get = urllib2.Request(keepalive_endpoint)
        urllib2.urlopen(get, timeout=5)
        time.sleep(5)


def push_update():
    db_endpoint = "http://" + SERVER_IP + ":" + str(SERVER_PORT) + "/arduino-to-db"

    value = bridgeclient()
    global io_state

    while RUN:
        post = False

        for io in io_state:
            new_val = value.get(io)
            old_val = io_state.get(io)
            if new_val != old_val:
                io_state.update({io: new_val})
                post = True
            time.sleep(0.05)

        if post:
            print "POSTING\n", io_state, "\nTO ENDPOINT ", db_endpoint
            try:
                io_state_json =  json.dumps(io_state)
                post = urllib2.Request(db_endpoint, io_state_json, {'Content-Type': 'application/json'})
                resp = urllib2.urlopen(post, timeout=5)
                print resp.read()
            except urllib2.HTTPError, e:
                print 'HTTPError = ' + str(e.code)
            except urllib2.URLError, e:
                print 'URLError = ' + str(e.reason)
            except Exception:
                import traceback
                print 'generic exception: ' + traceback.format_exc()


class RestHTTPRequestHandler(BaseHTTPRequestHandler):
    
    @staticmethod
    def parse_query(path):
        parsed_path = urlparse.urlparse(path)
        queries = parsed_path.query.split("&")
        query_dict = {}
        for q in queries:
            key, value = q.split("=")
            query_dict[key] = value
        return query_dict

    def do_GET(self):
        global SERVER_IP
        global SERVER_PORT
        if SERVER_IP is not None or SERVER_PORT is not None:
            try:
                cmd = urllib.unquote(self.path.split("/")[1]) + "\n"
                ATmega_socket.send(cmd)
                self.send_response(200)
                self.end_headers()
            except Exception as e:
                print e
        else:
            try:
                queries = self.parse_query(self.path)
                print queries
                SERVER_IP = queries["ip_address"]
                SERVER_PORT = queries["port"]

                t1 = threading.Thread(target=push_update)
                t2 = threading.Thread(target=keep_alive)
                t1.start()
                t2.start()

                self.send_response(200)
                self.end_headers()
            except Exception as e:
                self.send_response(417)
                self.end_headers()
                print e

        return

    # def do_POST(self):
    #     new_id = max(filter(lambda x: x['id'], TODOS))['id'] + 1
    #     form = cgi.FieldStorage(fp=self.rfile,
    #                        headers=self.headers, environ={
    #                             'REQUEST_METHOD':'POST', 
    #                             'CONTENT_TYPE':self.headers['Content-Type']
    #                        })
    #     new_title = form['title'].value
    #     new_todo = {'id': new_id, 'title': new_title}
    #     TODOS.append(new_todo)
 
    #     self.send_response(201)
    #     self.end_headers()
    #     self.wfile.write(json.dumps(new_todo))
    #     return
 
httpd = HTTPServer(('0.0.0.0', 9898), RestHTTPRequestHandler)
while True:
    httpd.handle_request()

RUN = False
