import os
import threading

SH = '../bin/CMPsim.usetrace.32 -threads 1 -t ../traces/%s -o %s -cache UL3:1024:64:16 -LLCrepl %d'

alg_id = 3
alg_name = 'output_SECRU'

def run(fi, fo):
    os.system(SH % (fi, fo, alg_id, ))

if __name__ == '__main__':
    th = []
    os.mkdir(alg_name)
    for line in open('files.txt', 'r'):
        fi = line.replace('\n', '')
        fo = alg_name + '/' + fi.replace('out.trace.gz', 'stats')
        t = threading.Thread(target=run, args=(fi, fo, ))
        t.start()
        th.append(t)
        if len(th) == 7:
            th[0].join()
            th = th[1:]
    for t in th:
        t.join()

