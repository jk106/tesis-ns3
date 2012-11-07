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
#ifndef MIH_HELPER_H
#define MIH_HELPER_H

#include <string>

#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/deprecated.h"
#include "ns3/trace-helper.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/network-manager.h"


namespace ns3 {

class Packet;

/**
 * \brief build a set of MihNetDevice objects
 *
 * Normally we eschew multiple inheritance, however, the classes 
 * PcapUserHelperForDevice and AsciiTraceUserHelperForDevice are
 * treated as "mixins".  A mixin is a self-contained class that
 * encapsulates a general attribute or a set of functionality that
 * may be of interest to many other classes.
 */
class MihHelper : public PcapHelperForDevice, public AsciiTraceHelperForDevice
{
public:
  /**
   * Construct a MihHelper.
   */
  MihHelper ();
  virtual ~MihHelper () {}

  

  /**
   * This method creates an ns3::CsmaChannel with the attributes configured by
   * CsmaHelper::SetChannelAttribute, an ns3::CsmaNetDevice with the attributes
   * configured by CsmaHelper::SetDeviceAttribute and then adds the device
   * to the node and attaches the channel to the device.
   *
   * \param node The node to install the device in
   * \returns A container holding the added net device.
   */
  NetDeviceContainer Install (Ptr<Node> node) const;

  /**
   * This method creates an ns3::CsmaChannel with the attributes configured by
   * CsmaHelper::SetChannelAttribute, an ns3::CsmaNetDevice with the attributes
   * configured by CsmaHelper::SetDeviceAttribute and then adds the device
   * to the node and attaches the channel to the device.
   *
   * \param name The name of the node to install the device in
   * \returns A container holding the added net device.
   */
  NetDeviceContainer Install (std::string name) const;

  
  NetDeviceContainer Install (const NodeContainer &c) const;
  void Activate (Ptr<Node> n,uint8_t index, bool dep);
  void SetNetId (Ptr<Node> n,uint8_t netid);
  void SetAddress (Ptr<Node> n,Ipv4Address addr);
  void SetQoS(Ptr<Node> n, uint8_t qos);
  void SetNetworkManager(Ptr<Node> n,Ptr<NetworkManager> netman);


private:
  /*
   * \internal
   */
  Ptr<NetDevice> InstallPriv (Ptr<Node> node) const;

  /**
   * \brief Enable pcap output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, 
                                    std::string prefix, 
                                    Ptr<NetDevice> nd,
                                    bool explicitFilename);

  ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif /* CSMA_HELPER_H */
