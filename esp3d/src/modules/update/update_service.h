/*
  update_service.h -  update services functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _UPDATE_SERVICES_H
#define _UPDATE_SERVICES_H

class UpdateService {
 public:
  UpdateService();
  ~UpdateService();
  void handle();
  bool begin();
  void end();

 private:
  bool flash(const char* filename, int type);
};

extern UpdateService update_service;

#endif  //_UPDATE_SERVICES_H
