cplusplus {{
    #include <vector>
    typedef std::vector<int> IntVector;
}};
class noncobject IntVector;

enum ALL_MSGS
{
  APP_MSG     = 1; 
  DATA_MSG    = 2; 
  FC_MSG      = 3;
  SW_FLAG     = 4;
  SETUP_MSG   = 5;
  BUFLEN_MSG  = 6;
  PATHLEN_MSG = 7;
}
message appMsg extends cPacket
{
    int srcLid;
    int dstLid;
    int msgId;
    int flitLength;//flit数量
    int pathLength; //组间路径长度
}


message dataMsg extends cPacket
{
    int srcLid;
    int dstLid;
    int msgId;
    int flitSn;
    int flitLength;//flit数量
    int pathLength;  //组间路径长度
    simtime_t curTime;
}


message SWFLAG extends cPacket//电交换机端口传输完成flag
{
    int outPortNum;
}


message setupMsg extends cPacket
{
    int MEMSID;
    IntVector configuration;
    bool flag;//标志位
    bool ini;
    

}


message BufferLength extends cPacket
{
    int srcGroupIdx;
    IntVector bufLen;
}

message pathLength extends cPacket
{
	double recPathLength;
}

