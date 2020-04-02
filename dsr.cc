#include "ns3/dsr-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "ns3/ip-l4-protocol.h"
using namespace ns3;
using namespace dsr;
int packetsSent = 0;
int packetsReceived = 0;

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
    std::cout<< "socket Receive - " <<socket->Recv () <<" Received packet"<<packet<<std::endl;
	  packetsReceived++;
      std::cout<<"Received packet - "<<packetsReceived<<" and Size is "<<packet->GetSize ()<<" Bytes."<<std::endl;
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      packetsSent++;
      std::cout<<"Packet sent - "<<packetsSent<<std::endl;
ReceivePacket ( socket);
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount-1, pktInterval);

    }
  else
    {
      socket->Close ();
    }
}

int main(int argc, char **argv)
{
  uint32_t size=14;
  double step=100;
  double totalTime=100;

  int packetSize = 1024;
  int totalPackets = totalTime-1;
  double interval = 1.0;
  Time interPacketInterval = Seconds (interval);

  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);

  MobilityHelper mobilityAdhoc;
  mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                               "MinX", DoubleValue (0.0),
                               "MinY", DoubleValue (0.0),
                               "DeltaX", DoubleValue (500),
                               "DeltaY", DoubleValue (500),
                               "GridWidth", UintegerValue (5),
                               "LayoutType", StringValue ("RowFirst"));
  mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobilityAdhoc.Install (nodes);
int jump=1,jump1=0,jump2=1;
  for (uint n=0 ; n < nodes.GetN() ; n++)
   {
     if(n<4){
      Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0, 0, 0));
      mob->SetPosition(Vector(jump, 1.15, 0));
      jump++;

    }
    if(n>=4&&n<9){
     Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0, 0, 0));
     mob->SetPosition(Vector(jump1, 2.3, 0));
        jump1+=2;
   }
   if(n>=9){
    Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0, 0, 0));
    mob->SetPosition(Vector(jump2, 3.3, 0));
     jump2++;
  }
   }

  WifiMacHelper wifiMac ;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi ;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  DsrMainHelper dsrMain;
  DsrHelper dsr;
  InternetStackHelper stack;
  stack.Install (nodes);
  dsrMain.Install (dsr, nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);






// //Ptr<ns3::dsr::DsrRouting> dsrrout13=dsr.Create (nodes.Get(13));
// ObjectFactory m_agentFactory;
// m_agentFactory.SetTypeId ("ns3::dsr::DsrRouting");
// Ptr<ns3::dsr::DsrRouting> agent = m_agentFactory.Create<ns3::dsr::DsrRouting> ();
// // deal with the downtargets, install UdpL4Protocol, TcpL4Protocol, Icmpv4L4Protocol
// Ptr<UdpL4Protocol> udp = nodes.Get(0)->GetObject<UdpL4Protocol> ();
// agent->SetDownTarget (udp->GetDownTarget ());
// udp->SetDownTarget (MakeCallback (&dsr::DsrRouting::Send, agent));
//
// //DsrRouting::PacketNewRoute ( packet,interfaces.GetAddress(0), interfaces.GetAddress(13), 48);
// //                             Ipv4Address source,
// //                             Ipv4Address destination,
// //                             uint8_t protocol)


  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (0), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (1), tid);
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  // Tracing
  wifiPhy.EnablePcap ("wifi-simple-infra", devices);

  // Output what we are doing
  Simulator::Schedule (Seconds (1), &GenerateTraffic, source, packetSize, totalPackets, interPacketInterval);






  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  AnimationInterface anim ("Secure-DSR-in-MANET/dsr-output.xml");

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();


  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/dsr-flow.xml", true, true);
  Simulator::Destroy ();

  std::cout<<"\n\n***** OUTPUT *****\n\n";
  std::cout<<"Total Packets sent = "<<packetsSent<<std::endl;
  std::cout<<"Total Packets received = "<<packetsReceived<<std::endl;
  std::cout<<"Packet delivery ratio = "<<(float)(packetsReceived/packetsSent)*100<<" %"<<std::endl;

}
