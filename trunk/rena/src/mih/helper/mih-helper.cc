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

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/object-factory.h"
#include "ns3/mih-net-device.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"

#include "ns3/trace-helper.h"
#include "mih-helper.h"

#include <string>
#include <vector>

NS_LOG_COMPONENT_DEFINE ("MihHelper");

namespace ns3 {

MihHelper::MihHelper ()
{
  m_deviceFactory.SetTypeId ("ns3::MihNetDevice");
}


void 
MihHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  
}

void 
MihHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  
}

NetDeviceContainer
MihHelper::Install (Ptr<Node> node) const
{
  return InstallPriv (node);
}

NetDeviceContainer
MihHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

NetDeviceContainer 
MihHelper::Install (const NodeContainer &c) const
{
  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add (InstallPriv (*i));
    }

  return devs;
}


Ptr<NetDevice>
MihHelper::InstallPriv (Ptr<Node> node) const
{
  std::vector<Ptr<NetDevice> > devs=node->GetNetDevices();
  Ptr<MihNetDevice> device = m_deviceFactory.Create<MihNetDevice> ();
  node->AddDevice (device);
  device->SetNetDevices(devs);
  return device;
}

void
MihHelper::Activate(Ptr<Node> n, uint8_t index)
{
  Ptr<NetDevice> dev=n ->GetDevice(n->GetNDevices()-1);
  NetDevice* apl=PeekPointer(dev);
  MihNetDevice* mih;
  mih = (MihNetDevice*) apl;
  mih->Activate(index);
}

void
MihHelper::SetAddress(Ptr<Node> n, Ipv4Address addr)
{
  Ptr<NetDevice> dev=n ->GetDevice(n->GetNDevices()-1);
  NetDevice* apl=PeekPointer(dev);
  MihNetDevice* mih;
  mih = (MihNetDevice*) apl;
  mih->SetAddress(addr);
}

} // namespace ns3
