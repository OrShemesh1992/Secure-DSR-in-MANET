#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/dsr-helper.h"
#include "ns3/dsr-main-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/dsr-rcache.h"
using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {

      Ptr<Packet> p = Create<Packet> (pktSize);
      std::cout << p->ToString()<<"checkkkkkkkkkk"<<std::endl;
      socket->Send (p);
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 1;
  uint32_t numNodes = 15;  // by default, 5x5
  uint32_t destNode = 9;
  uint32_t sourceNode = 14;
  double interval = 1.0; // seconds



  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

//***************************wifi*************************
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  // if (verbose)
    //{
      //wifi.EnableLogComponents ();  // Turn on all Wifi logging
     //}

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-65) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
  //***************************END**************************

//***************************SetPosition*************************
MobilityHelper mobilityAdhoc;
mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
                             "MinX", DoubleValue (0.0),
                             "MinY", DoubleValue (0.0),
                             "DeltaX", DoubleValue (distance),
                             "DeltaY", DoubleValue (distance),
                             "GridWidth", UintegerValue (5),
                             "LayoutType", StringValue ("RowFirst"));
mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
mobilityAdhoc.Install (c);
double jump=0.5,jump1=0.2,jump2=0.5;
for (uint n=0 ; n < c.GetN() ; n++)
 {
   if(n<4){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump, 1.15, 1));
    jump++;

  }
  if(n>=4&&n<9){
   Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
   mob->SetVelocity(Vector(0,0,0));
   mob->SetPosition(Vector(jump1, 3.45, 1));
      jump1+=2;
 }
 if(n>=9){
  Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
  mob->SetVelocity(Vector(0,0,0));
  mob->SetPosition(Vector(jump2, 5.75, 1));
   jump2++;
}
 }
  // MobilityHelper mobilityAdhoc;
  // mobilityAdhoc.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                              "MinX", DoubleValue (0.0),
  //                              "MinY", DoubleValue (0.0),
  //                              "DeltaX", DoubleValue (distance),
  //                              "DeltaY", DoubleValue (distance),
  //                              "GridWidth", UintegerValue (5),
  //                              "LayoutType", StringValue ("RowFirst"));
  // mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  // mobilityAdhoc.Install (c);
  //***************************END********************************


//***************************DSR********************************
  DsrHelper dsr;
  DsrMainHelper dsrMain;

  InternetStackHelper internet;
  internet.Install (c);
  dsrMain.Install (dsr, c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
//***************************END********************************
//***************************socket********************************
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (destNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (destNode, 0), 80);
  source->Connect (remote);
//***************************END***********************************

  // Give DSR time to converge-- 30 seconds perhaps
  Simulator::Schedule (Seconds (30.0), &GenerateTraffic,
                       source, packetSize, numPackets, interPacketInterval);

  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << destNode << " with grid distance " << distance);

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

  Simulator::Stop (Seconds (33.0));
  AnimationInterface anim ("Secure-DSR-in-MANET/dsr-output.xml");
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/dsr-flow.xml", true, true);
  Simulator::Destroy ();

  return 0;
}
