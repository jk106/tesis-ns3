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
#include "network-manager.h"
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



NS_LOG_COMPONENT_DEFINE ("NetworkManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NetworkManager);

TypeId 
NetworkManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetworkManager")
    .AddConstructor<NetworkManager> ()
  ;
  return tid;
}

NetworkManager::NetworkManager ()
{
}

void
NetworkManager::AddNetChart(NetChart *chart, uint8_t path)
{
  m_netcharts.push_back(chart);
  m_paths.push_back(path);
}

void
NetworkManager::RequestPSol(uint8_t netchartid, MihNetDevice *device,uint8_t tech_old,uint8_t tech_new)
{
  Ptr<NetChart> one;
  Ptr<NetChart> two;
  if(tech_new!=4)
{
  for(uint8_t i=0;i<m_netcharts.size();i++)
  {
    Ptr<NetChart> dev=m_netcharts[i];
    if(dev->GetId()==netchartid && dev->GetTechnology()==tech_old)
    {
      one=dev;
      break;
    }
  }
  //one->RemoveRouting(device->GetMihAddress());
}
if(tech_new==4)
{
  uint8_t j=0;
  for(j=0;j<m_netcharts.size();j++)
  {
    Ptr<NetChart> dev=m_netcharts[j];
    if(dev->GetId()==netchartid && dev->GetTechnology()==tech_old)
    {
      two=dev;
      break;
    }
  }
  two->AddRouting(device->GetMihAddress());
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[j]);
}
else if(tech_new!=0)
{
  uint8_t j=0;
  for(j=0;j<m_netcharts.size();j++)
  {
    Ptr<NetChart> dev=m_netcharts[j];
    if(dev->GetId()==netchartid && dev->GetTechnology()==tech_new)
    {
      two=dev;
      break;
    }
  }
  two->RemoveRouting(device->GetMihAddress());
  two->AddRouting(device->GetMihAddress());
  device->Activate(tech_new);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
  remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
  remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[j]);
}
else
{
//Check for the closest and less congested network
//First Distance
  uint8_t j=0;
  int8_t sel=-1;
  double min_distance=1000000;//Arbitrary large distance
  for(j=0;j<m_netcharts.size();j++)
  {
    Ptr<NetChart> dev=m_netcharts[j];
    Vector pos=device->GetNode()->GetObject<MobilityModel>()->GetPosition();
    Vector ap=dev->GetApLocation();
    double xx=pos.x-ap.x;
    double yy=pos.y-ap.y;
    double zz=pos.z-ap.z;
    double distance=sqrt(xx*xx+yy*yy+zz*zz);
    uint8_t tech= dev-> GetTechnology();
    if((distance<min_distance)&&!((tech_old==dev->GetTechnology())&&(device->GetNetId()==dev->GetId())))
    {
      if((tech==2 && distance<700)||(tech==1 && distance<110)||(tech==3 && distance<2000))//Check that closest AP is not out of tech range
      {
        min_distance=distance;
        sel=j;
      }
    }
  }
  if(sel!=-1)
  {
    two=m_netcharts[sel];
    two->AddRouting(device->GetMihAddress());
    device->Activate(two->GetTechnology());
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
    remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
    remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[sel]);
  }
}
}

void
NetworkManager::NotifyPack(uint8_t netchartid_old, uint8_t netchartid_new)
{
  
}

void
NetworkManager::NotifyPNack(uint8_t netchartid)
{
  
}

void
NetworkManager::SetLma(Ptr<Node> node)
{
  lma=node;
}

void
NetworkManager::DoDispose (void)
{
  Object::DoDispose();
}

} // namespace ns3
