/*
 * p_test.cc
 *
 * Test using pthreads to double-buffer I/O. Use like this:
 *      ./p_test -T -m -r 5 <big file> -o <output file>
 * to test regular sequential reads and writes. Use like this:
 *      ./p_test -t -T -m -r 5 <big file> -o <output file>
 * to test running the calls to write() in a child thread.
 *
 * the options -T and -m turn on timing and 'messages'; -r N
 * make p_test read and write <big file> N times
 *
 *  Created on: Aug 31, 2015
 *      Author: jimg
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "MarshallerThread.h"

using namespace std;
using namespace libdap;

#define CPP_STREAMS 1

static double time_diff_to_hundredths(struct timeval *stop, struct timeval *start)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (stop->tv_usec < start->tv_usec) {
        int nsec = (start->tv_usec - stop->tv_usec) / 1000000 + 1;
        start->tv_usec -= 1000000 * nsec;
        start->tv_sec += nsec;
    }
    if (stop->tv_usec - start->tv_usec > 1000000) {
        int nsec = (start->tv_usec - stop->tv_usec) / 1000000;
        start->tv_usec += 1000000 * nsec;
        start->tv_sec -= nsec;
    }

    double result = stop->tv_sec - start->tv_sec;
    result += double(stop->tv_usec - start->tv_usec) / 1000000;
    return result;
}

int main(int argc, char *argv[])
{
    int repetitions = 1;
    bool threads = false;
    bool print_time = false;
    bool messages = false;
    string outfile = "";
    char ch;

    while ((ch = getopt(argc, argv, "tr:Tmo:")) != -1) {
        switch (ch) {
        case 't':
            threads = true;
            break;
        case 'T':
            print_time = true;
            break;
        case 'm':
            messages = true;
            break;
        case 'r':
            repetitions = atoi(optarg);
            break;
        case 'o':
            outfile = optarg;
            break;
        case '?':
        default:
            cerr << "p_test [-t(hreads)T(ime)m(essages)] [-r <reps>] [-o <out>] <file>" << endl;
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

    vector<char> buf(bytes + 1);

    MarshallerThread *mt = new MarshallerThread();
    if (print_time)
        mt->set_print_time(true);

    // To simulate the open stream/network, open once.
    if (outfile.empty()) {
        outfile = infile;
        outfile.append(".out");
    }

#if CPP_STREAMS
    ofstream out(outfile.c_str(), ios::out);
    if (!out || out.fail()) {
        cerr << "Failed to open file!" << endl;
        return 1;
    }
#else
    int out_file = open(outfile.c_str(), O_WRONLY|O_CREAT|O_CLOEXEC|O_APPEND|O_TRUNC, 0664);
    if (out_file == -1) {
        cerr << "Failed to open file!" << endl;
        return 1;
    }
#endif
    for (int i = 0; i < repetitions; ++i) {
        {
            // to simulate our problem, open and read for each iteration
            if (messages) cerr << "open and read..." << endl;

            struct timeval tp_s;
            if (print_time && gettimeofday(&tp_s, 0) != 0) cerr << "could not read time" << endl;
#if CPP_STREAMS
            ifstream in(infile.c_str(), ios::in);
            in.read(&buf[0], bytes);
#else
            int in_file = open(infile.c_str(), O_RDONLY);
            int bytes_read = read(in_file, &buf[0], bytes);
            if (bytes_read != bytes)
                cerr << "error reading bytes using file descriptor" << endl;
#endif
            struct timeval tp_e;
            if (print_time) {
                if (gettimeofday(&tp_e, 0) != 0) cerr << "could not read time" << endl;

                cerr << "time to read: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
            }
        }

        {
            char *tmp = new char[bytes + 1];
            memcpy(tmp, &buf[0], bytes);

            if (threads) {
                if (messages) cerr << "writing using a child thread..." << endl;

                struct timeval tp_s;
                if (print_time && gettimeofday(&tp_s, 0) != 0) cerr << "could not read time" << endl;

                Locker lock(mt->get_mutex(), mt->get_cond(), mt->get_child_thread_count());
                mt->increment_child_thread_count();
                // thread deletes tmp
#if CPP_STREAMS
                mt->start_thread(libdap::MarshallerThread::write_thread, out, tmp, bytes);
#else
                mt->start_thread(libdap::MarshallerThread::write_thread, out_file, tmp, bytes);
#endif
                struct timeval tp_e;
                if (print_time) {
                    if (gettimeofday(&tp_e, 0) != 0) cerr << "could not read time" << endl;

                    cerr << "time to fork thread: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
                }
            }
            else {
                if (messages) cerr << "writing sequentially..." << endl;

                struct timeval tp_s;
                if (print_time && gettimeofday(&tp_s, 0) != 0) cerr << "could not read time" << endl;

#if CPP_STREAMS
                out.write(tmp, bytes);
#else
                int bytes_written = write(out_file, tmp, bytes);
                if (bytes_written != bytes)
                    cerr << "error writing bytes using a file descriptor" << endl;
#endif
                delete tmp;
                tmp = 0;

                struct timeval tp_e;
                if (print_time) {
                    if (gettimeofday(&tp_e, 0) != 0) cerr << "could not read time" << endl;

                    cerr << "time to write: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
                }

            }
        }
    }

#if CPP_STREAMS
    // No need to close the ofstream object
#else
    close(out_file);
#endif
    delete mt;

    return 0;
}

