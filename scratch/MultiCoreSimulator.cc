/*
 * File  :      MultiCoreSim.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 15, 2020
 */
#include "ns3/core-module.h"
#include "ns3/tinyxml.h"
#include "ns3/MCoreSimProjectXml.h"
#include "ns3/MCoreSimProject.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("MultiCoreSimulator");

int main (int argc, char *argv[])
{
  // simulator output Trace Files
  string CpuTraceFile       = "CpuAckTrace";   // Dump CPU Ack data
  string CohCtrlsTraceFile  = "CohCtrlsTrace"; // Dump Internal states of 
                                               // CPU Mem Controllers

  // simulator input files
  string SimConfigFile = "test_cfg.xml"; // Configuration Parameter File
  string BMsPath = "BMs/tests/";         // Benchmark TraceFiles Path

  bool LogFileGenEnable = true;          // enable log file dumps

  // command line parser
  CommandLine cmd;

  // adding a call to our sim configurable parameters 
  cmd.AddValue("CfgFile", "simulator configuration file", SimConfigFile);
  cmd.AddValue("BMsPath", "benchmark trace file(s) path", BMsPath);
  cmd.AddValue("BusTraceFile", "trace file for bus transactions", CpuTraceFile);
  cmd.AddValue("CtrlsTraceFile", "trace file for coherence controlles", CohCtrlsTraceFile);
  cmd.AddValue("LogFileGenEnable", "enable flag for log file dump", LogFileGenEnable);

  // parse user commands
  cmd.Parse (argc, argv);

  // read Xml Configuration File
  TiXmlDocument doc(SimConfigFile.c_str());
  doc.LoadFile();
  TiXmlHandle hDoc(&doc);
  TiXmlElement* root = hDoc.FirstChildElement().Element();
  TiXmlHandle hroot = TiXmlHandle(root);
  MCoreSimProjectXml xml;
  xml.LoadFromXml          ( hroot            );
  xml.SetBMsPath           ( BMsPath          );
  xml.SetCohCtrlsTraceFile ( CohCtrlsTraceFile);
  xml.SetCpuTraceFile      ( CpuTraceFile     );
  xml.SetLogFileGenEnable  ( LogFileGenEnable );

  // setup simulation environment
  MCoreSimProject project   (xml              );


  // set simulation clock to one nano-Second
  // clock resolution is the smallest time value 
  // that can be respresented in our simulator
  Time::SetResolution (Time::NS); // MS, US, PS

  // lunch simulator
  project.Start();
  cout<<"Reza: In  scratch/MultiCoreSimulator after setting paths and project.start()  "<<endl;
  Simulator::Run();
  cout<<"Reza: Run finished "<<endl;
  // clean up once done
  Simulator::Destroy();
  return 0;

}
