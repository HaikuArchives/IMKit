/*
 * Select (Helper) class - handles list of file descriptors to make
 * select calls a little easier and C++ier.
 *
 * Limitations: Doesn't support more than one of READ/WRITE/EXCEPTION
 * on a particular descriptor at once (the disconnection code isn't
 * intelligent enough to)
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#include <map>
#include <sigc++/signal_system.h>

#ifndef EXAMPLE_SOCKET_H
#define EXAMPLE_SOCKET_H

// ------------------------------------------------------------------
// Select class
// ------------------------------------------------------------------

class Select {
 public:
  // enums
  enum SocketInputCondition
  {
    Read      = 1 << 0,
    Write     = 1 << 1,
    Exception = 1 << 2
  };
  // this enum is used as a 'bitmask' of the input conditions

  // typedefs
  typedef SigC::Slot2<void,int,SocketInputCondition> SlotType;
  typedef SigC::Callback2<void,int,SocketInputCondition> Callback;

 private:
  // the lists of file descriptors (ok they're sets.. you get the idea)
  typedef std::map<int, Callback*> SocketMap;
  SocketMap rfdl, wfdl, efdl;

 public:
  Select();

  /**
   * This method will register a callback for a given file descriptor,
   * on condition. The SigC::Slot sd will be called.
   */
  SigC::Connection connect(const SlotType &sd, int source,
			   SocketInputCondition condition);

  /**
   * Execute the select on the sockets, until one has an event, or
   * until the interval specified expires.
   *
   * @param maximum time before returning (in milliseconds)
   * @return boolean, true if the timeout was hit, false otherwise
   */
  bool run(unsigned int interval = 0);

  /**
   *  For internal use only.
   */
  void _disconnect(int fd);
};

class SelectSigCNode : public SigC::SlotNode
{
 private:
  Select *m_parent;
  int m_fd;

 public:
  SelectSigCNode(Select *parent, int fd);
  virtual ~SelectSigCNode();
};

#endif // EXAMPLE_SOCKET_H
