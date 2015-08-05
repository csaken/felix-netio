#!/usr/bin/env python

import subprocess
import re
import csv

from contextlib import contextmanager

@contextmanager
def server(bin):
        p = subprocess.Popen(bin)
        yield
        p.kill()


def run_benchmark_libevent(msgsize=64):
        with server(['../x86_64-slc6-gcc47-opt/netio_server', '-p', str(13000+msgsize)]):
                out = subprocess.check_output(['../x86_64-slc6-gcc47-opt/netio_datagen', '-s', 
                        str(msgsize), '-p', str(13000+msgsize), '-n', str(10**6/msgsize)])
        m = re.search(r"Overall: ([0-9\.]+) MB/s", out)
        return m.group(1)

def run_benchmark_asyncmsg(msgsize=64):
        with server(['../x86_64-slc6-gcc47-opt/netio_server', '-a', '-p', str(12000+msgsize)]):
                out = subprocess.check_output(['../x86_64-slc6-gcc47-opt/netio_datagen', '-a', 
                        '-s', str(msgsize), '-p', str(12000+msgsize), '-n', str(10**6/msgsize)])
        m = re.search(r"Overall: ([0-9\.]+) MB/s", out)
        return m.group(1)


def run_all(sizes):
        e, a = dict(), dict()
        for s in sizes:
                print s
                e[s] = run_benchmark_libevent(s)
                a[s] = run_benchmark_asyncmsg(s)
        return e, a

def write_results(results_libevent, results_asyncmsg):
        with open('results_asyncmsg_vs_libevent.csv', 'w') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(["Size", "Throughput libevent [MB/s]", "Throughput asyncmsg [MB/s]"])
                for size in sorted(results_libevent.keys()):
                        writer.writerow([str(size), str(results_libevent[size]), str(results_asyncmsg[size])])


if __name__ == '__main__':
        e, a = run_all([2**i for i in range(4, 16)])
        write_results(e, a)
