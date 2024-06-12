import os
import time

command_add = 'git add .'
command_commit = 'git commit -m "Auto commit %TIME%"' # %TIME% va fi inlocuit cu unixu
command_push = 'git push origin master'

# delete scores.txt (sunt personale)
os.system('del scores.txt')

# execute comenzile
os.system(command_add)
os.system(command_commit.replace('%TIME%', str(int(time.time()))))
os.system(command_push)

# pauza iar
# print("Press Enter to exit...")
# input()

# run line_counter.py
os.system('python line_counter.py') # puteam si cu import dar neh