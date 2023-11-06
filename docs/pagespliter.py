#!/usr/bin/env python3

import os

input_file = "espXXX.md"
output_file_head = "esp"
output_ext = ".md"


   
f = open(input_file, "r")
lines = f.readlines()
#parse each lines
head = ""
have_command = False
fo=0
count = 0
for line in lines:
    if not have_command:
      count=0
      head+=line
      if line.startswith("title ="):
        have_command = True
        num = line.split("ESP")[1].split("]")[0]
        fo = open(output_file_head+num+output_ext, "w")
        fo.write(head)
        have_command = True
    else:    
          if line.startswith("+++"):
              count+=1
              if count==2:
                fo.close()
                have_command = False
                head = line
                print("close file "+output_file_head+num+output_ext)
              else:
                fo.write(line)
          else:
              fo.write(line)
         
f.close()