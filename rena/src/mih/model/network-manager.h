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
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "ns3/object.h"
#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include "ns3/traced-callback.h"
#include <vector>
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/mobility-module.h"
#include "netchart.h"




namespace ns3 {

class SimpleChannel;
class Node;

/**
 * \ingroup netdevice
 *
 * This device does not have a helper and assumes 48-bit mac addressing;
 * the default address assigned to each device is zero, so you must 
 * assign a real address to use it.  There is also the possibility to
 * add an ErrorModel if you want to force losses on the device.
 * 
 * \brief simple net device for simple things and testing
 */
class NetworkManager : public Object
{
public:
  static TypeId GetTypeId (void);
  NetworkManager();

  void AddNetChart(NetChart chart, uint8_t path);
  void RequestPSol(uint8_t netchartid, uint8_t nodeid,uint8_t tech_old,uint8_t tech_new);
  void NotifyPack(uint8_t netchartid_old, uint8_t netchartid_new);
  void NotifyPNack(uint8_t netchartid);
  void SetLma(Ptr<Node> node, Ipv4Address addr);  

protected:
  virtual void DoDispose (void);
private:
  Ptr<Node> lma;//LMA
  std::vector<NetChart> m_netcharts;//The interfaces of each node downstream
  std::vector<int> m_paths;//The interfaces for each NetChart from the LMA
};

} // namespace ns3

#endif /* SIMPLE_NET_DEVICE_H */
