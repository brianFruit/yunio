#!/usr/bin/python

"""
Copyright (C) 2017-2018 Brian Xia
You may use, distribute and modify this code under the
terms of the license, which is included in the project
root directory.
"""

import urllib2
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
# Endpoint to receive Arduino I/O values
URL = "http://[server_ip_address]:port/path/"

ATmega_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ATmega_socket.connect(('localhost', 6571))

io_state = {}
for io in range(NUMIO):
    io_state[str(io)] = None

def pushUpdate():
    value = bridgeclient() 
    global io_state

    run = True
    while run:
        post = False
        for io in io_state:
            new_val = value.get(io)
            old_val = io_state.get(io)
            if new_val != old_val:
                io_state.update({io: new_val})
                post = True
            time.sleep(0.05)

        if post:
            print io_state
            try:
                io_state_json =  json.dumps(io_state)
                post = urllib2.Request(URL, io_state_json, {'Content-Type': 'application/json'})
                resp = urllib2.urlopen(post)
                print resp.read()
            except urllib2.HTTPError, e:
                print 'HTTPError = ' + str(e.code)
            except urllib2.URLError, e:
                print 'URLError = ' + str(e.reason)
            except Exception:
                import traceback
                print 'generic exception: ' + traceback.format_exc()
                
        # run = False


# Thread to constantly update I/O pin values
t = threading.Thread(target=pushUpdate)
t.start()


# Light weight server to listen for command and then route to ATmega
class RestHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        cmd = self.path.split("/")[1] + "\n"
        ATmega_socket.send(cmd)
        self.send_response(200)
        self.end_headers()
        return

 
httpd = HTTPServer(('0.0.0.0', 9898), RestHTTPRequestHandler)
while True:
    httpd.handle_request()
