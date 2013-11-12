import os, sys

def run(cmd):
  exit_code = os.system(cmd)
  if exit_code:
    sys.stderr.write('Command "%s" failed with exit code %s.\n' % (cmd, exit_code))
    sys.exit(exit_code)

def main():
  my_path = os.path.dirname(os.path.abspath(__file__))
  old_cwd = os.getcwd()
  os.chdir(my_path)
  try:
    target = '../../build/zeromq/libzmq.a'
    if not os.path.isfile(target):
      if not os.path.isfile('configure'):
        run('autoconf')
      if not os.path.isfile('Makefile'):
        run('./configure --with-pgm')
      src = 'src/.libs/libzmq.a'
      if not os.path.isfile(src):
        run('make -j6')
      run('cp %s %s' % (src, target))
  finally:
    os.chdir(old_cwd)
    
if __name__ == '__main__':
  main()

# vim: set ts=2 sw=2 expandtab:
