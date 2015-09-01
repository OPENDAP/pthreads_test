/*
 * p_test.cc
 *
 *  Created on: Aug 31, 2015
 *      Author: jimg
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>

#include "MarshallerThread.h"

using namespace std;
using namespace libdap;

#undef TIME 
#define MSG(x)
//#define MSG(x) do { x; } while(false)

double time_diff_to_hundredths(struct timeval *stop, struct timeval *start)
{
  /* Perform the carry for the later subtraction by updating y. */
  if( stop->tv_usec < start->tv_usec )
    {
      int nsec = (start->tv_usec - stop->tv_usec) / 1000000 + 1 ;
      start->tv_usec -= 1000000 * nsec ;
      start->tv_sec += nsec ;
    }
  if( stop->tv_usec - start->tv_usec > 1000000 )
    {
      int nsec = (start->tv_usec - stop->tv_usec) / 1000000 ;
      start->tv_usec += 1000000 * nsec ;
      start->tv_sec -= nsec ;
    }

  double result =  stop->tv_sec - start->tv_sec;
  result += (stop->tv_usec - start->tv_usec) / 1000000;
  return result;
}

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
  	    MSG(cerr << "open and read..." << endl);
#if TIME
	    struct timeval tp_s;
	    if (gettimeofday(&tp_s, 0) != 0)
	      cerr << "could not read time" << endl;
#endif
            ifstream in(infile.c_str(), ios::in);
            in.read(&buf[0], bytes);
#if TIME
	    struct timeval tp_e;
	    if (gettimeofday(&tp_e, 0) != 0)
	      cerr << "could not read time" << endl;

	    cerr << "time to read: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
#endif
        }

        {
            char *tmp = new char[bytes+1];
            memcpy(tmp, &buf[0], bytes);

            if (threads) {
	        MSG(cerr << "writing using a child thread..." << endl);
#if TIME
		struct timeval tp_s;
		if (gettimeofday(&tp_s, 0) != 0)
		    cerr << "could not read time" << endl;
#endif
                Locker lock(mt->get_mutex(), mt->get_cond(), mt->get_child_thread_count());
                mt->increment_child_thread_count();
                // thread deletes tmp
                mt->start_thread(libdap::MarshallerThread::write_thread, out, tmp, bytes);
#if TIME
		struct timeval tp_e;
		if (gettimeofday(&tp_e, 0) != 0)
		   cerr << "could not read time" << endl;

		cerr << "time to start thread: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
#endif
            }
            else {
	        MSG(cerr << "writing sequentially..." << endl);
#if TIME
		struct timeval tp_s;
		if (gettimeofday(&tp_s, 0) != 0)
		    cerr << "could not read time" << endl;
#endif
                out.write(tmp, bytes);
                delete tmp; tmp = 0;
#if TIME
		struct timeval tp_e;
		if (gettimeofday(&tp_e, 0) != 0)
		    cerr << "could not read time" << endl;

		cerr << "time to write: " << time_diff_to_hundredths(&tp_e, &tp_s) << endl;
#endif
            }
        }
    }

    delete mt;

    return 0;
}

