#!/usr/bin/env python

import sys, os
import SimpleHTTPServer, SocketServer

#Replace this with a different path if you need to...
base_path = os.path.join(os.getcwd(),"ESP8266","data")

class MyHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path.endswith(".tpl"):
            self.send_response(301)
            self.send_header("Content-type", "text/html")
            self.end_headers()

            data = self.process(self.path)
            self.wfile.write(data)
            self.wfile.close()
            return
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    def process(self,fn):
        if fn.startswith("/") or fn.startswith("\\"):
            fn = fn[1:]
        fn = os.path.join(base_path,os.path.normpath(fn))
        fpath,_ = os.path.split(fn)
        lines = open(fn).read()
        lines = lines.split("\n")
        n_lines = len(lines)
        i = 0
        while i < n_lines: 
            line = lines[i].strip()
            if line.startswith("$INCLUDE["):
                p0 = line.find("[")+1
                p1 = line.find("]")
                if p0 < 0 or p0 == len(line) or p1 < 0:
                    continue
                fn_inc = os.path.join(fpath,line[p0:p1])
                if not os.path.exists(fn_inc):
                    i = i+1
                    continue

                lines_inc = open(fn_inc).read()
                lines_inc = lines_inc.split("\n")
                if i < n_lines-1:
                    lines1 = lines[i+1:]
                else:
                    lines1 = []
                lines = lines[:i]+lines_inc+lines1
                n_lines = len(lines)
            else:
                i = i+1
        return "\n".join(lines)

if __name__ == '__main__':
    print "="*60
    print "Serving files from:"
    print base_path
    os.chdir(base_path)
    print "="*60
    handler = MyHandler
    server = SocketServer.TCPServer(("",8080), handler)
    server.serve_forever()
