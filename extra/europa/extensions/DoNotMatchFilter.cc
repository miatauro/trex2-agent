/*********************************************************************
 * Software License Agreement (BSD License)
 * 
 *  Copyright (c) 2011, MBARI.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 * 
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TREX Project nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "DoNotMatchFilter.hh"

#include "Debug.hh"
#include "tinyxml.h"

using namespace TREX::europa;

DoNotMatchFilter::DoNotMatchFilter(EUROPA::TiXmlElement const &cfg)
:EUROPA::SOLVERS::FlawFilter(cfg, true) {
  debugMsg("trex:not_match", "Creating not match filter");
  for(EUROPA::TiXmlElement *child=cfg.FirstChildElement("Choice");
      NULL!=child; child = child->NextSiblingElement("Choice")) {
    char const *name = child->Attribute("name");
    if( NULL!=name ) {
      debugMsg("trex:not_match", " accept: "<<name);
      m_names.insert(name);
    }
  }
  if( m_names.empty() )
    debugMsg("trex:always", "No choice name => this filter will always exclude.")
}

bool DoNotMatchFilter::test(EUROPA::EntityId const &entity) {
  bool ret = m_names.find(entity->getName())==m_names.end();
  debugMsg("trex:not_match", "Testing for variable "<<entity->getName()<<": "<<ret);
  return ret;
}
