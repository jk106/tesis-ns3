/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "netchart.h"
#include "ns3/simple-channel.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/error-model.h"
#include "ns3/mobility-module.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/mih-tag.h"
#include "ns3/vector.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include <vector>



NS_LOG_COMPONENT_DEFINE ("NetChart");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NetChart);

TypeId 
NetChart::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetChart")
    .AddConstructor<NetChart> ()
  ;
  return tid;
}

NetChart::NetChart ()
  : m_id (0),
  m_connections(0)
{
}

void
NetChart::SetNodes(std::vector<Ptr<Node> > &c, std::vector<int> &d)
{
  for(unsigned int i=0; i<c.size();i++)
  {
     Ptr<Node> dev= c[i];
     m_nodes.push_back(dev);
  }
  for(unsigned int i=0; i<d.size();i++)
  {
     int dev= d[i];
     m_indexesdown.push_back(dev);
  }
  NS_ASSERT_MSG(m_indexesdown.size()==m_nodes.size(),"The number of nodes and interfaces is not equal!");
}

void
NetChart::SetId(uint8_t id)
{
  m_id=id;
}

uint8_t
NetChart::GetId()
{
  return m_id;
}

void
NetChart::SetTechnology(uint8_t tech)
{
  m_tech=tech;
}

uint8_t
NetChart::GetTechnology()
{
  return m_tech;
}

void
NetChart::SetApLocation(Vector a)
{
  m_aplocation=a;
}

Vector
NetChart::GetApLocation()
{
  return m_aplocation;
}

void
NetChart::AddRouting(Ipv4Address ipv4a, int n)
{
  for(uint8_t i=0;i<m_nodes.size();i++)
  {
    Ptr<Node> dev=m_nodes[i];
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (dev->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (ipv4a, Ipv4Mask ("255.255.255.0"), m_indexesdown[i]);
  }
  m_subs.push_back(n);
  m_connections++;
}

void
NetChart::RemoveRouting(Ipv4Address ipv4a, int n)
{
  for(uint8_t i=0;i<m_nodes.size();i++)
  {
    Ptr<Node> dev=m_nodes[i];
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (dev->GetObject<Ipv4> ());
    remoteHostStaticRouting->RemoveStaticRoute (ipv4a);
  }
  for(uint8_t i=0;i<m_subs.size();i++)
  {
    if(m_subs.at(i)==n){ 
      m_subs.erase(m_subs.begin()+i);
      break;
    }
  }
  m_connections--;
}

void
NetChart::RemoveSub(int n)
{
std::cout<<"NetChart::RemoveSub "<<n<<std::endl;
  for(uint8_t i=0;i<m_subs.size();i++)
  {
    if(m_subs.at(i)==n){ 
      m_subs.erase(m_subs.begin()+i);
      break;
    }
  }
  m_connections--;
}

std::vector<int>
NetChart::GetSubs()
{
  return m_subs;
}
 
void
NetChart::DoDispose (void)
{
  m_id=0;
  Object::DoDispose();
}

} // namespace ns3
