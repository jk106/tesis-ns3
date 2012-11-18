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
#include "ns3/node-list.h"




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

bool
NetworkManager::RequestPSol(uint8_t netchartid, MihNetDevice *device,uint8_t tech_old,uint8_t tech_new)
{
std::cout<<"PSOL"<<(int)netchartid<<(int)tech_old<<(int)tech_new<<device->GetNode()->GetId()<<std::endl;
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
  two->AddRouting(device->GetMihAddress(), device->GetNode()->GetId());
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[j]);
  return true;
}
else if(tech_new!=0)
{
  uint8_t j=0;
  int sele=-1;    
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
    //std::cout<<"Distance: "<<(double)distance<<" TECH: "<<(uint8_t) tech<<std::endl;
    if(((tech==2 && distance<700)||(tech==1 && distance<110)||(tech==3 && distance<2000) )&& tech==tech_new)//Check that closest AP is not out of tech range
    {
      two=dev;
      sele=dev->GetId();
      break;
    }
    
  }
  if(sele!=-1)
  {
    std::vector<int> vec=two->GetSubs();
    if((vec.size()<2&&tech_new==1)||(vec.size()<5&&tech_new==2)||tech_new==3)
    {
      two->RemoveRouting(device->GetMihAddress(),device->GetNode()->GetId());
      two->AddRouting(device->GetMihAddress(),device->GetNode()->GetId());
      Simulator::Schedule(Seconds(0.05),&MihNetDevice::Activate,device,tech_new);
      //device->Activate(tech_new);
      device->UpdateNetId(two->GetId());
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
      remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
      remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[j]);
      one->RemoveSub(device->GetNode()->GetId());
      return true;
    }
    else
    {
      int min=100;
      MihNetDevice* ho;
      for(unsigned int k=0;k<vec.size();k++)
      {
        Ptr<Node> node = NodeList::GetNode (vec[k]);
        Ptr<NetDevice> dev=node ->GetDevice(node->GetNDevices()-1);
        NetDevice* apl=PeekPointer(dev);
        MihNetDevice* mih;
        mih = (MihNetDevice*) apl;
        if(mih)
        {
          //std::cout<<"MIHNetDevice found on node: "<<vec[k]<<" with qos: "<<(int) mih->GetQos()<<std::endl;
          if(mih->GetQos()<min)
          {
            min=mih->GetQos();
            ho=mih;
          }
        }
      }
      if(min<device->GetQos())
      {
        if(NIHandover(ho,tech_new,two->GetId()))
        {
          two->RemoveRouting(device->GetMihAddress(),device->GetNode()->GetId());
          two->AddRouting(device->GetMihAddress(),device->GetNode()->GetId());
          Simulator::Schedule(Seconds(0.05),&MihNetDevice::Activate,device,tech_new);
          //device->Activate(tech_new);
          device->UpdateNetId(two->GetId());
          Ipv4StaticRoutingHelper ipv4RoutingHelper;
          Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
          remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
          remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[j]);
          one->RemoveSub(device->GetNode()->GetId());
          return true;
        }
      }
    }
    
  }
  return false;
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
    two->AddRouting(device->GetMihAddress(),device->GetNode()->GetId());
    Simulator::Schedule(Seconds(0.05),&MihNetDevice::Activate,device,two->GetTechnology());
    //device->Activate(two->GetTechnology());
    device->UpdateNetId(two->GetId());
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
    remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
    remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[sel]);
    one->RemoveSub(device->GetNode()->GetId());
    return true;
  }
  return false;
}
}

bool
NetworkManager::NIHandover(MihNetDevice *device,uint8_t tech_old, uint8_t netchartid)
{
std::cout<<"NIHandover"<<device->GetNode()->GetId()<<std::endl;
  uint8_t j=0;
  int8_t sel=-1;
  double min_distance=1000000;//Arbitrary large distance
  Ptr<NetChart> one;
  Ptr<NetChart> two;
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
    two->AddRouting(device->GetMihAddress(),device->GetNode()->GetId());
    Simulator::Schedule(Seconds(0.05),&MihNetDevice::Activate,device,two->GetTechnology());
    //device->Activate(two->GetTechnology());
    std::cout<<"NIHandover to "<<(int)two->GetTechnology()<<std::endl;
    device->UpdateNetId(two->GetId());
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (lma->GetObject<Ipv4> ());
    remoteHostStaticRouting->RemoveStaticRoute (device->GetMihAddress());
    remoteHostStaticRouting->AddNetworkRouteTo (device->GetMihAddress(), Ipv4Mask ("255.0.0.0"), m_paths[sel]);
    for(uint8_t i=0;i<m_netcharts.size();i++)
    {
      Ptr<NetChart> dev=m_netcharts[i];
      if(dev->GetId()==netchartid && dev->GetTechnology()==tech_old)
      {
        one=dev;
        break;
      }
    }
    one->RemoveSub(device->GetNode()->GetId());
    Simulator::Schedule(Seconds(200),&NetworkManager::RequestPSol,this,two->GetId(),device,two->GetTechnology(),0);
    return true;
  }
  return false;
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
