#!/usr/bin/python2
import os
import time
import fcntl
import subprocess
import re
import sys
import time
import glob

class IPopen(subprocess.Popen):
    def __init__(self, *args, **kwargs):
        keyword_args = {
            'stdin': subprocess.PIPE,
            'stdout': subprocess.PIPE,
            'stderr': subprocess.PIPE,
            'prompts': [],
        }
        keyword_args.update(kwargs)
        self.prompts = keyword_args.get('prompts')
        del keyword_args['prompts']
        subprocess.Popen.__init__(self, *args, **keyword_args)
        for outfile in (self.stdout, self.stderr):
            if outfile is not None:
                fd = outfile.fileno()
                fl = fcntl.fcntl(fd, fcntl.F_GETFL)
                fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

    def correspond(self, text, sleep=0.1):
        self.stdin.write(text)
        self.stdin.flush()
        str_buffer = ''
        while 1:
            try:
                str_buffer += self.stdout.read()
                for prompt in self.prompts:
                    if prompt in str_buffer:
                        return str_buffer
            except IOError:
                time.sleep(sleep)

def test():
    for binary in glob.glob(os.path.join(sys.argv[2], '*.elf')):
        print 'Testing', binary

        # Extract post conditions
        post = []
        with open(binary[:-4] + '.S', 'r') as f:
            post_mode = False
            for line in f.readlines():
                if line.strip() == '# Post':
                    post_mode = True
                    continue
                if not line.startswith('#') and post_mode:
                    break
                if post_mode:
                    post.append(line[1:].strip())

        # Print extracted post conditions
        if post:
            print 'Using following post conditions:'
            for p in post:
                print '\t', p
        else:
            print 'WARNING : NO POST CONDITIONS FOR:', binary

        # Run
        args = [
            sys.argv[1] if '/' in sys.argv[1] else './' + sys.argv[1],
            'dummy.cfg.old',
            binary
        ]
        prc = IPopen(args, prompts=['Press enter', 'Failed', 'ra = '])
        text = prc.correspond('\n'*10)
        out = text
        while 'Press enter' in text:
            if 'Press enter' in text:
                try:
                    text = prc.correspond('\n'*10)
                    out += text
                except IOError:
                    break

        # Test post conditions
        for p in post:
            if p not in text:
                print 'Post condition:', p, 'Failed'
                print 'See log.txt for full output'
                with open('log.txt', 'w') as f:
                    f.write(out)
                return
        print ''

    print 'All tests passed'
    print 'You are winner!!!'

if len(sys.argv) != 3:
    print 'Usage: ' + sys.argv[0] + ' SimulatorPath TestFolder'
    exit(0)

with open('dummy.cfg.old', 'w') as f:
    for _ in range(8):
        f.write('0\n')
    f.write('8 8 8\n')
    f.write('8 8 8\n')
    f.write('8 8 8\n')

test()
os.remove('dummy.cfg.old')
