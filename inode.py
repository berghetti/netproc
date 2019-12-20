import os

def find_pid(inode):

    # get a list of all files and directories in /proc
    procFiles = os.listdir("/proc/")

    # remove the pid of the current python process
    procFiles.remove(str(os.getpid()))

    # set up a list object to store valid pids
    pids = []

    for f in procFiles:
        try:
            # convert the filename to an integer and back, saving the result to a list
            integer = int(f)
            pids.append(str(integer))
        except ValueError:
            # if the filename doesn't convert to an integer, it's not a pid, and we don't care about it
            pass

    for pid in pids:
        # check the fd directory for socket information
        fds = os.listdir("/proc/%s/fd/" % pid)
        for fd in fds:
            # save the pid for sockets matching our inode
            if ('socket:[%d]' % inode) == os.readlink("/proc/%s/fd/%s" % (pid, fd)):
                return pid

def main():
    pid = find_pid(20741)
    print (pid)

if __name__ == '__main__':
    main()
