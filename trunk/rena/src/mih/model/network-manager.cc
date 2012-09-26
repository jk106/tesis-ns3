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
NetworkManager::AddNetChart(NetChart chart, uint8_t path)
{
  m_netcharts.push_back(chart);
  m_paths.push_back(path);
}

void
NetworkManager::RequestPSol(uint8_t netchartid, uint8_t nodeid,uint8_t tech_old,uint8_t tech_new)
{
  
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
NetworkManager::SetLma(Ptr<Node> node, Ipv4Address addr)
{
  lma=node;
}

void
NetworkManager::DoDispose (void)
{
  Object::DoDispose();
}

} // namespace ns3
