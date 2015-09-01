/*
 * p_test.cc
 *
 *  Created on: Aug 31, 2015
 *      Author: jimg
 */

#include "config.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <unistd.h>

#include "MarshallerThread.h"

using namespace std;
using namespace libdap;

int
main(int argc, char *argv[])
{
    int repetitions = 1;
    bool threads = false;
    char ch;

    while ((ch = getopt(argc, argv, "tr:")) != -1) {
            switch (ch) {
            case 't':
                    threads = true;
                    break;
            case 'r':
                repetitions = atoi(optarg);
                    break;
            case '?':
            default:
                    cerr << "p_test [rt] <file>" << endl;
                    return 1;
            }
    }
    argc -= optind;
    argv += optind;

    string infile;

    if (argc == 1) {
        infile = argv[0];
    }
    else {
        cerr << "wrongs number of arguments." << endl;
        return 1;
    }

    cerr << "File name: " << infile << ", Repetitions: " << repetitions << endl;

    // read the file
    struct stat statbuf;
    int status = stat(infile.c_str(), &statbuf);
    if (status != 0) {
        cerr << "failed to read stat buf" << endl;
        return 1;
    }
    long bytes = statbuf.st_size;
    cerr << "file is " << bytes << " bytes." << endl;

    vector<char> buf(bytes+1);

    MarshallerThread *mt = new MarshallerThread();

    // To simulate the open stream/network, open once.
    string outfile = infile;
    outfile.append(".out");
    ofstream out(outfile.c_str(), ios::out);

    for (int i = 0; i < repetitions; ++i) {
        {
            // to simulate our problem, open and read for each iteration
            cerr << "open and read..." << endl;

            ifstream in(infile.c_str(), ios::in);
            in.read(&buf[0], bytes);
        }

        {
            char *tmp = new char[bytes+1];
            memcpy(tmp, &buf[0], bytes);

            if (threads) {
                cerr << "writing using a child thread..." << endl;

                Locker lock(mt->get_mutex(), mt->get_cond(), mt->get_child_thread_count());
                mt->increment_child_thread_count();
                // thread deletes tmp
                mt->start_thread(libdap::MarshallerThread::write_thread, out, tmp, bytes);
            }
            else {
                cerr << "writing sequentially..." << endl;

                out.write(tmp, bytes);
                delete tmp; tmp = 0;
            }
        }
    }

    delete mt;

    return 0;
}

