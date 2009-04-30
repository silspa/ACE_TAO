// -*- C++ -*-

//=============================================================================
/**
 *  @file    schedulability_check.cpp
 *
 *  $Id$
 *
 *  @author  Friedhelm Wolf (fwolf@dre.vanderbilt.edu)
 */
//=============================================================================

#include <fstream>
#include <sstream>
#include <iostream>
#include <numeric>
#include "Scheduler.h"
#include "CTT_Enhanced.h"
#include "CTT_Basic.h"
#include <ace/Get_Opt.h>

std::string filename = "test.sd"; // filename of task list input
bool counting_mode = false;
bool average_mode = false;
bool check_overbooking = false;
unsigned int consistency_level = 0;

class ScheduleChecker : public std::binary_function <double,
                                                     SCHEDULE::value_type,
                                                     double>
{
public:
  ScheduleChecker (const SCHEDULE & schedule)
    : schedule_ (schedule)
  {
    // build replica_groups data structure
    for (SCHEDULE::const_iterator sched_it = schedule.begin ();
         sched_it != schedule.end ();
         ++sched_it)
      {
        Processor processor = sched_it->first;
        for (TASK_LIST::const_iterator task_it = sched_it->second.begin ();
             task_it != sched_it->second.end ();
             ++task_it)
          {
            REPLICA_GROUPS::iterator rep_it = 
              replica_groups_.find (primary_name (*task_it));

            TASK_POSITION position (processor, *task_it);
            if (rep_it == replica_groups_.end ())
              {
                TASK_POSITIONS entry;
                entry.push_back (position);
                replica_groups_[primary_name (*task_it)] = entry;
              }
            else
              {
                replica_groups_[
                  primary_name (*task_it)].push_back (position);
              }
          }
      }
  }

  double operator () (double previous, const SCHEDULE::value_type & entry)
  {
    PROCESSOR_SETS scenarios = 
      this->calculate_failure_scenarions (entry.first,
                                          this->max_rank ());

    return .0;
  }

private:
  PROCESSOR_SETS calculate_failure_scenarions (const Processor & processor,
                                               unsigned int consistency_level)
  {
    PROCESSOR_SETS result;
    PROCESSOR_LIST all_processors = get_processors (schedule_, true);

    return result;
  }

  unsigned int max_rank (void)
  {
    unsigned int max_rank = 0;

    for (SCHEDULE::const_iterator sched_it = schedule_.begin ();
         sched_it != schedule_.end ();
         ++sched_it)
      {
        for (TASK_LIST::const_iterator task_it = sched_it->second.begin ();
             task_it != sched_it->second.end ();
             ++task_it)
          {
            max_rank = std::max (max_rank,
                                 extract_rank (task_it->name));
          }
      }

    return max_rank;
  }

private:
  const SCHEDULE & schedule_;
  REPLICA_GROUPS replica_groups_;
};

static int
parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opt (argc, argv, ACE_TEXT ("acf:ho"));

  int c;

  while ((c = get_opt ()) != -1)
    switch (c)
    {
    case 'f':
      filename = get_opt.opt_arg ();
      break;
    case 'c':
      counting_mode = true;
      break;
    case 'a':
      average_mode = true;
      break;
    case 'o':
      check_overbooking = true;
      break;
    case 'h':
    default:
      std::cerr << "usage: scheck "
                << "\t-a" << std::endl
                << "\t-c" << std::endl
                << "\t-f <filename>" << std::endl;
      return 1;
  }

  return 0;
}


int main (int argc, char *argv[])
{
  if (parse_args (argc, argv) != 0)
    return 1;

  std::ifstream ifile;
  ifile.open (filename.c_str ());

  SCHEDULE schedule = read_schedule (ifile);
  
  ifile.close ();

  if (counting_mode)
    {
      std::cout << processor_usage (schedule) << std::endl;
    }
  else if (average_mode)
    {
      for (SCHEDULE::iterator it = schedule.begin ();
           it != schedule.end ();
           ++it)
        {
          if (it->first.compare ("Unknown") != 0)
            std::cout << it->first << ":" << std::endl;

          std::list <double> utilization;

          for (TASK_LIST::iterator t_it = it->second.begin ();
               t_it != it->second.end ();
               ++t_it)
            {
              utilization.push_back (t_it->execution_time / t_it->period);
            }

          std::cout << "minimum utilization = " 
                    << *std::min_element (utilization.begin (),
                                          utilization.end ()) * 100 << " %"
                    << std::endl;

          std::cout << "maximum utilization = " 
                    << *std::max_element (utilization.begin (),
                                          utilization.end ()) * 100 << " %"
                    << std::endl;

          std::cout << "average utilization = " 
                    << (std::accumulate (utilization.begin (),
                                         utilization.end (),
                                         0.0) / utilization.size ()) * 100 << " %"
                    << std::endl;
          
        }
    }
  else
    {
      unsigned long usage = processor_usage (schedule);

      SCHEDULE min_schedule;
      SCHEDULE::const_iterator schedule_it = schedule.begin ();
      for (unsigned long i = 0; i < usage; ++i)
        {
          min_schedule.insert (*schedule_it);
        }

      std::map <Processor, double> wcrts;

      double wcrt = std::accumulate (min_schedule.begin (),
                                     min_schedule.end (),
                                     -1.0,
                                     ScheduleChecker (min_schedule));
      
      return wcrt > .0;
    } // end else counting_mode not true

  return 0;
}
