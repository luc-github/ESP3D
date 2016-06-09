#!/usr/bin/env python
#original file and idea come from https://github.com/j0hnlittle

import SimpleHTTPServer, SocketServer
import sys
import os
import json
import re

#Replace this with a different path if you need to...
base_path = os.path.join(os.getcwd(),"..","esp3d","data")
tools_path = os.getcwd();
server_port=8080

class MyHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        is_tpl, s = self.process_path(self.path)
        if is_tpl:
            self.send_response(301)
            self.send_header("Content-type", "text/html")
            self.end_headers()

            data = self.process_tpl(s)
            self.wfile.write(data)
            self.wfile.close()
            return
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    def process_path(self,s):
        #A template link is all caps and is associated to a lower().tpl file in base_path
        if s == "/":
            return True,"home.tpl"

        ret = False,""
        s = s.replace("/","")
        if s.endswith(".tpl"):
           return True,s

        s = s.lower()+".tpl"

        #these do not exactly match, so let's make them!
        s = s.replace("configsta","config_sta") 
        s = s.replace("configap","config_ap") 
        s = s.replace("configsys","system") 

        if os.path.exists(os.path.join(base_path,s)):
            ret = True,s

        return ret

    def process_tpl(self,fn):
        p = re.compile('\$(.*?)\$')
        if fn.startswith("/") or fn.startswith("\\"):
            fn = fn[1:]

        fn = os.path.join(base_path,fn)
        data = open(fn).read()

        fn_json = os.path.join(tools_path,"tags.json")
        if os.path.exists(fn_json):
            json_dic = json.loads(open(fn_json).read())
        else:
            json_dic = {}

        tags = p.findall(data)
        i = 0
        n_tags = len(tags)
        while i < n_tags:
            dd = self.process_tag(data,tags[i],json_dic)
            if dd != data:
                data = dd
                tags = p.findall(data)
                n_tags = len(tags)
            else:
                i = i+1
        return data

    def process_tag(self, data, tag, json_dic={}):
        print "  processing $%s$" % tag
        if tag in json_dic.keys():
            data = data.replace("$"+tag+"$",json_dic[tag])
        elif tag.startswith("INCLUDE[") and tag.endswith("]"):
            fn = tag[8:-1]
            fn = os.path.join(base_path,fn)
            d = open(fn).read()
            p0 = data.find("$"+tag)
            p1 = data.find("]",p0)+2
            data = data[:p0]+d+data[p1:]

        return data

if __name__ == '__main__':
    print "="*60
    print "Serving files from:"
    print base_path
    print "\ntags.json is located at:"
    print tools_path
    print "\nOpen your browser at http://localhost:" + str(server_port)
    
    os.chdir(base_path)
    print "="*60
    handler = MyHandler
    server = SocketServer.TCPServer(("",server_port), handler)
    server.serve_forever()

