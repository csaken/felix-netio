#!/usr/bin/env python

import subprocess
import re
import csv

def run_benchmark(msgsize=64):
        out = subprocess.check_output(['x86_64-slc6-gcc47-opt/netio_datagen', '-s', str(msgsize)])
        m = re.search(r"Overall: ([0-9\.]+) MB/s", out)
        return m.group(1)

def run_netperf(msgsize=64):
        out = subprocess.check_output(['netperf', '-H', 'localhost', '-f', 'M', '--', '-m', str(msgsize)])
        return out.splitlines()[-1].split()[-1]


def run_all(sizes):
        d = dict()
        n = dict()
        for s in sizes:
                print s
                d[s] = run_benchmark(s)
                n[s] = run_netperf(s)
        return d, n

def write_results(results, netperf):
        with open('netio_performance_localhost.csv', 'w') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(["Size", "Throughput [MB/s]", "Netperf [MB/s]"])
                for size in sorted(results.keys()):
                        writer.writerow([str(size), str(results[size]), str(netperf[size])])


if __name__ == '__main__':
        d, n = run_all([2**i for i in range(4, 16)])
        write_results(d, n)
