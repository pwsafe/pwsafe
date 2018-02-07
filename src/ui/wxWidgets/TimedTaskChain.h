/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

////////////////////////////////////////////////////////////////////
// TimedTaskChain.h - A timer driven single-threaded task chain,
// somewhat like JavaScript promises. Each task is kicked off on receiving
// a timer event. The current default is 100ms
//
// Basically, you can do this
//
// new TimedTaskChain( {task1, task2, task3...} )
// 		.then( task )
// 		.then( task )
// 		.then( task )
// 		...
// 		.error( error_handler_functor );
//
// Each task is only invoked after the next timer event has been received
// effectively ensuring that the current iteration of event loop has unwound.
// Error handler (if set) is called as soon as an error happens.
//
// The purpose is similar to that of wxEvtHandler::CallAfter(), but
// I have seen at least one instance where CallAfter didn't help:
//
// #1178 Linux autotype fails when "Minimize after Autotype" is not checked
//
// Somehow, pwsafe needs time to minimize itself, and not just unwind from
// the current iteration of event loop. There could be other instances where
// You might really need to wait for a finite amount of time, and not just
// post a callback event to yourself.
//
// Note that the TimedTaskChain object must be created on the heap, so that
// it does not get destroyed as the current call stack unwinds. Similarly,
// it cannot be destructed at will. It will autodestruct once it finishes
// executing all its tasks. Therefore, the ctor and dtor are private, and
// it must be constructed via a public static function.
//
// Each task is a functor (regular function, lambda or a functor) that takes
// and returns void. The error handler functor should take a std::exception
// derived class as argument. It is only invoked if any of the functions in
// the task chain throws, in which case the rest of the tasks in the chain
// are not invoked.

#ifndef __TIMEDTASKCHAIN_H__
#define __TIMEDTASKCHAIN_H__

#include <wx/timer.h>

#include <list>
#include <functional>

class TimedTaskChain: public wxTimer
{
	typedef std::function<void(void)> TaskType;
  typedef std::pair<TaskType, int> TaskWithInterval;
	typedef std::list<TaskWithInterval> TaskList;

	typedef std::function<void(const std::exception&)> ErrorHandlerType;

	ErrorHandlerType m_errorHandler;
	TaskList m_tasks;
	TaskList::iterator m_nextTask;

	TimedTaskChain(std::initializer_list<TaskWithInterval> tasks);
	TimedTaskChain(std::initializer_list<TaskType> tasks);
    ~TimedTaskChain() {}

    static int DefaultTaskDelay();

public:
	static TimedTaskChain& CreateTaskChain(std::initializer_list<TaskWithInterval> tasks);
  static TimedTaskChain& CreateTaskChain(std::initializer_list<TaskType> tasks);
  static TimedTaskChain& CreateTaskChain(const TaskType &tasks);

	TimedTaskChain& then(const TaskType& task, int delay = DefaultTaskDelay() ) { m_tasks.push_back({task, delay}); return *this; }

	void OnError(ErrorHandlerType errHandler) { m_errorHandler = errHandler; }

    // overridden from wxTimer
    void Notify();

private:
	void RunTask();
	void Next();
};

#endif
