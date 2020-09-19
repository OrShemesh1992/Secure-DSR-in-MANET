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
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include "ns3/dsdv-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/animation-interface.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
using namespace ns3;
using namespace std;


NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

class Experiment{

//functions of the class
public:
std::string m_protocolName;
string toplogyName;
std::string m_CSVfileName;
Experiment (uint32_t protocol, uint32_t topology);
void ReceivePacket (Ptr<Socket> socket);
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval );
void Run (std::string CSVfileName);
std::string CommandSetup (int argc, char **argv);
void CheckThroughput ();
void lines (NodeContainer c);
void circle (NodeContainer c);
void grid (NodeContainer c);
void locateOnCircle (NodeContainer c, std::set<pair<double,double>> points);
double circleEquasion(double x, double y, double center);
double randomDouble(double low, double high);
void square (NodeContainer c);
void random(NodeContainer c);
void setNames();

//members of the class
private:
  std::string phyMode = "DsssRate1Mbps";
  double distance; // m
  uint32_t packetSize; // bytes
  uint32_t numPackets;
  uint32_t numNodes;  // by default, 5x5
  map<int, int> packetsRecievedPerNode;
  double interval;// seconds
  int recvPackets;
  uint32_t m_protocol;
  int position;
  vector<int> destNodes;
  vector<int> sourceNodes;
  std::vector<Ptr<Socket>> recieves;
  std::vector<Ptr<Socket>> sources;

};

//initalize constructor
Experiment::Experiment (uint32_t protocol, uint32_t topology){
    distance = 1000;  // m
    packetSize = 1000; // bytes
    numPackets = 2000;
    numNodes = 50;  // by default, 5x5
    interval = 0.5; // seconds
    //m_CSVfileName = "Dsr_project.csv";
    recvPackets = 0;
    m_protocol = protocol; // 1-olsr, 2-aodv, 3-DSR
    position = topology; //1-lines, 2-circle, 3-grid, 4-square, 5 -random
    destNodes = {1, 5 ,7 ,9};
    sourceNodes = { 3, 6, 8, 10};
    setNames();
}

//writing data to the csv file
void Experiment::setNames(){
  switch (position)
    {
    case 1:
      toplogyName = "lines";
      break;
    case 2:
      toplogyName = "circle";
      break;
    case 3:
      toplogyName = "grid";
      break;
    case 4:
      toplogyName = "square";
      break;
    case 5:
      toplogyName = "random";
      break;
    default:
      NS_FATAL_ERROR ("No such pos:" << position);
    }
    switch (m_protocol)
      {
        case 1:
          m_protocolName = "OLSR";
          break;
        case 2:
          m_protocolName = "AODV";
          break;
        case 3:
          m_protocolName = "DSR";
          break;
        default:
          NS_FATAL_ERROR ("No such protocol:" << m_protocol);
      }
}

 void
 Experiment::CheckThroughput ()
 {

   std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

   out << (Simulator::Now ()).GetSeconds () << ","
       << numNodes << ","
       << distance << ","
       << numPackets << ","
       << recvPackets << ","
       << m_protocolName << ","
       << std::endl;

   out.close ();
   //packetsReceived = 0;
   Simulator::Schedule (Seconds (1.0), &Experiment::CheckThroughput, this);
 }

//Receive packet
void
Experiment:: ReceivePacket (Ptr<Socket> socket)
{
   while (socket->Recv ())
     {
       int node = socket->GetNode()->GetId();
       packetsRecievedPerNode[node]++;
       //std::cout <<node<< " received a packet" << '\n';
     }
}

//sending a packet
void Experiment:: GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {

      Ptr<Packet> p = Create<Packet> (pktSize);
      //std::cout << p->ToString()<<"check"<<std::endl;
      socket->Send (p);
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

std::string
Experiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  //cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  //cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
  cmd.Parse (argc, argv);
  return m_CSVfileName;
}

int main (int argc, char *argv[])
{
  for (size_t i = 1; i <= 3; i++) {
    for (size_t j = 1; j <= 5; j++) {
      for (size_t k = 0; k < 1; k++) {
    /* code */

        Experiment experiment(i,j);
        //std::string CSVfileName = experiment.CommandSetup (argc,argv);
        string fileName =  experiment.m_protocolName + " " + experiment.toplogyName+".csv";
        experiment.m_CSVfileName = fileName;
      //writing the coloums to csv file
        std::ofstream out (fileName.c_str ());
        out << "seconds," <<
        "NumberOfNodes," <<
        "distance," <<
        "NumOfPackets," <<
        "RecievedPackets," <<
        "Protocol," <<
        std::endl;
        out.close ();
        experiment.Run (fileName);
      //std::cout <<"round"<<i+1<<":  ***********************************" <<recvPackets << '\n';
      }
    }
  }
}

void Experiment::lines (NodeContainer c){
  double jump=0.5,jump1=0.2,jump2=0.5;
  uint size = (c.GetN())/3;
  for (uint n=0 ; n < c.GetN() ; n++){
     if(n < size){
      Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0,0,0));
      mob->SetPosition(Vector(jump, 1.15, 1));
      jump++;

    }
    if(n >=size && n<= size*2){
     Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0,0,0));
     mob->SetPosition(Vector(jump1, 3.45, 1));
        jump1++;
   }
   if(n > size*2){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump2, 5.75, 1));
    jump2++;
   }
  }
}

void Experiment::circle (NodeContainer c) {
// Circle equasion: (x-a)**2 + (y-b)**2 = R**2
  bool setNotFull = true;
  double center = 2, radius = 2;
  int size = c.GetN(), low=0, high=4;
  std::set<pair<double,double>> points;
  while (setNotFull) {
    double x = randomDouble(low,high);
    double y = randomDouble(low,high);
    int res = circleEquasion(x,y,center);
    if(res == radius*radius){
      pair <double, double> p(x,y);
      points.insert(p);
    }
    int pointsSize = points.size();
    if(pointsSize == size){
      locateOnCircle(c, points);
      setNotFull = false;
    }
  }
}

double Experiment::circleEquasion(double x, double y, double center){
  double res = (x-center)*(x-center)+(y-center)*(y-center);
  return res;
}

void Experiment::locateOnCircle (NodeContainer c, std::set<pair<double,double>> points){
  int ind = 0;
  for (auto i: points) {
    Ptr<ConstantVelocityMobilityModel> m = c.Get(ind)->GetObject<ConstantVelocityMobilityModel>();
    m->SetVelocity(Vector(0,0,0));
    m->SetPosition(Vector(i.first,i.second, 1));
    ind++;
  }
}

double Experiment::randomDouble(double low, double high){
  int range=(high-low)+1;
  return range * (rand() / (RAND_MAX + 1.0));
}


void Experiment::grid (NodeContainer c){
  for (uint n=0 ; n < c.GetN() ; n++)
   {
     Ptr<ConstantVelocityMobilityModel> p = c.Get(n)->GetObject<ConstantVelocityMobilityModel> ();
   }
}

void Experiment::square (NodeContainer c){
  double jump=0.5,jump1=0.5,jump2=0.5, jump3=0.5;
  uint size = c.GetN()/4;
  //std::cout<<size<<std::endl;
  for (uint n=0 ; n < c.GetN() ; n++)
   {
     //left vertical edge
     if(n < size){
      Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
      mob->SetVelocity(Vector(0,0,0));
      mob->SetPosition(Vector(0, jump, 1));
      jump+=1.5;
    }
    //right vertical edge
    if(n >=size && n< size*2){
     Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
     mob->SetVelocity(Vector(0,0,0));
     mob->SetPosition(Vector(jump, jump1, 1));
        jump1+=1.5;
   }
   //lower horizental edge
   if(n >= size*2 && n<size*3){
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump2, jump1, 1));
     jump2+=1.5;
  }
  //upper horizental edge
   if(n >= size*3) {
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(jump3, 0, 1));
     jump3+=1.5;
   }
 }
}

void Experiment::random(NodeContainer c){
  int low = 0, high = 10;
  std::set<pair<double,double>> points;
  while(true){
    double x = randomDouble(low,high), y = randomDouble(low,high);
    pair <double, double> p(x,y);
    points.insert(p);
  //  int pointsSize = points.size();
    if(points.size() == c.GetN()) break;
  }
  int ind = 0;
  for (auto i: points) {
    Ptr<ConstantVelocityMobilityModel> mob = c.Get(ind)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetVelocity(Vector(0,0,0));
    mob->SetPosition(Vector(i.first, i.second, 1));
    ind++;
  }
}

void
Experiment::Run (std::string CSVfileName)
{

  Packet::EnablePrinting ();
  m_CSVfileName = CSVfileName;

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

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-57) );
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
                             "DeltaX", DoubleValue (2),
                             "DeltaY", DoubleValue (2),
                             "GridWidth", UintegerValue (numNodes/5),
                             "LayoutType", StringValue ("RowFirst"));

mobilityAdhoc.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");

mobilityAdhoc.Install (c);

switch (position)
  {
  case 1:
    lines(c);
    std::cout<< "This is lines topology"<<std::endl;
    break;
  case 2:
    circle(c);
    std::cout<< "This is circle topology"<<std::endl;
    break;
  case 3:
    grid(c);
    std::cout<< "This is grid topology"<<std::endl;
    break;
  case 4:
    square (c);
    std::cout<< "This is square topology"<<std::endl;
    break;
  case 5:
    random (c);
    std::cout<< "This is random topology"<<std::endl;
    break;
  default:
    NS_FATAL_ERROR ("No such pos:" << position);
  }

  //***************************END********************************


//***************************DSR********************************

   AodvHelper aodv;
   OlsrHelper olsr;
   DsrHelper dsr;
   DsrMainHelper dsrMain;
   Ipv4ListRoutingHelper list;
   InternetStackHelper internet;

   switch (m_protocol)
     {
     case 1:
       list.Add (olsr, 100);
       m_protocolName = "OLSR";
       break;
     case 2:
       list.Add (aodv, 100);
       m_protocolName = "AODV";
       break;
     case 3:
       m_protocolName = "DSR";
       break;
     default:
       NS_FATAL_ERROR ("No such protocol:" << m_protocol);
     }
     std::cout<< "This is "<<m_protocolName<<" protocol"<<std::endl;
   if (m_protocol < 3)
     {
       internet.SetRoutingHelper (list);
       internet.Install (c);
     }
   else if (m_protocol == 3)
     {
       internet.Install (c);
       dsrMain.Install (dsr, c);
     }



  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ip = ipv4.Assign (devices);

///********************************try - see if works!!!
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  std::vector<InetSocketAddress> remotes;
  for (size_t i = 0; i < destNodes.size(); i++) {
    Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (destNodes[i]), tid);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&Experiment::ReceivePacket, this));
    recieves.push_back(recvSink);
    Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNodes[i]), tid);
    InetSocketAddress remote = InetSocketAddress (ip.GetAddress (destNodes[i], 0), 80);
    source->Connect (remote);
    remotes.push_back(remote);
    sources.push_back(source);
    // Give DSR time to converge-- 30 seconds perhaps
    Simulator::Schedule (Seconds (1.0), &GenerateTraffic,
                         sources[i], packetSize, numPackets, interPacketInterval);
  }
//***************************END of try code***********************************


  // Output what we are doing
//  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << destNode << " with grid distance " << distance);

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

  CheckThroughput();

  Simulator::Stop (Seconds (100.0));
  AnimationInterface anim ("scratch/Dsr_project.xml");
  Simulator::Run ();
  for (auto itr = Experiment::packetsRecievedPerNode.begin(); itr != Experiment::packetsRecievedPerNode.end(); ++itr) {
          cout <<  "Number of packet received for node " << itr->first
               << " is: " << itr->second << '\n';
    }
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/dsr-flow.xml", true, true);
  Simulator::Destroy ();
}
