#include <iostream>
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/dsr-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"

 using namespace ns3;
 using namespace dsr;

 NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

 class RoutingExperiment
 {
 public:
   RoutingExperiment (); //constructor
   void Run (double txp);

 private:
   Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
   void ReceivePacket (Ptr<Socket> socket);

   uint32_t port;
   uint32_t bytesTotal;
   uint32_t packetsReceived;
   double m_txp;
 };

 RoutingExperiment::RoutingExperiment ()
   : port (9),
     bytesTotal (0),
     packetsReceived (0)
 {
 }

 void RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
 {
   Ptr<Packet> packet;
   Address senderAddress;
   while ((packet = socket->RecvFrom (senderAddress)))
     {
       bytesTotal += packet->GetSize ();
       packetsReceived += 1;
     }
 }


 Ptr<Socket> RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
 {
   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
   Ptr<Socket> sink = Socket::CreateSocket (node, tid);
   InetSocketAddress local = InetSocketAddress (addr, port);
   sink->Bind (local);
   sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

   return sink;
 }


 int main (int argc, char *argv[])
 {
   RoutingExperiment experiment;
   double txp = 7.5;
   experiment.Run (txp);
 }

 void RoutingExperiment::Run (double txp)
 {
   m_txp = txp;
   int nWifis = 3;

   double TotalTime = 200.0;
   std::string rate ("2048bps");
   std::string phyMode ("DsssRate11Mbps");
   // int nodeSpeed = 20;
   // int nodePause = 0;

   Config::SetDefault  ("ns3::OnOffApplication::PacketSize",StringValue ("64"));
   Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (rate));


   NodeContainer nodes;
   nodes.Create (nWifis);

   // setting up wifi phy and channel using helpers
   WifiHelper wifi;
   wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

   YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
   wifiPhy.SetChannel (wifiChannel.Create ());

   // Add a mac and disable rate control
   WifiMacHelper wifiMac;
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                 "DataMode",StringValue (phyMode),
                                 "ControlMode",StringValue (phyMode));

   wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
   wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

   wifiMac.SetType ("ns3::AdhocWifiMac");
   NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, nodes);

   MobilityHelper mobilityAdhoc;
   int64_t streamIndex = 5; // used to get consistent mobility across scenarios

   ObjectFactory pos;
   pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
   pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));
   pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));

   Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
   streamIndex += taPositionAlloc->AssignStreams (streamIndex);

   // std::stringstream ssSpeed;
   // ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
   // std::stringstream ssPause;
   // ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
   // mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
   //                                 "Speed", StringValue (ssSpeed.str ()),
   //                                 "Pause", StringValue (ssPause.str ()),
   //                                 "PositionAllocator", PointerValue (taPositionAlloc));
   mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
   mobilityAdhoc.Install (nodes);
   streamIndex += mobilityAdhoc.AssignStreams (nodes, streamIndex);
   NS_UNUSED (streamIndex); // From this point, streamIndex is unused


   DsrHelper dsr;
   DsrMainHelper dsrMain;
   InternetStackHelper internet;

       internet.Install (nodes);
       dsrMain.Install (dsr, nodes);

   NS_LOG_INFO ("assigning ip address");

   Ipv4AddressHelper addressAdhoc;
   addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer adhocInterfaces;
   adhocInterfaces = addressAdhoc.Assign (adhocDevices);

   OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address ());
   onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
   onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));


   for (int i = 0; i < 3 ; i++)
     {
       Ptr<Socket> sink = SetupPacketReceive (adhocInterfaces.GetAddress (i), nodes.Get (i));

       AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (i), port));
       onoff1.SetAttribute ("Remote", remoteAddress);

       Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
       ApplicationContainer temp = onoff1.Install (nodes.Get (i));
       temp.Start (Seconds (var->GetValue (100.0,101.0)));
       temp.Stop (Seconds (TotalTime));

     }

   Simulator::Stop (Seconds (TotalTime));
   AnimationInterface anim("anim1.xml");
   Simulator::Run ();
   Simulator::Destroy ();
 }
