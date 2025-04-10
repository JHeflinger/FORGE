#pragma once
#include "Simulation/Simulation.h"
#include <iostream>
#include <memory>
#include <string>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
	#define NOVIRTUALKEYCODES // VK_*
	#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
	#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
	#define NOSYSMETRICS      // SM_*
	#define NOMENUS           // MF_*
	#define NOICONS           // IDI_*
	#define NOKEYSTATES       // MK_*
	#define NOSYSCOMMANDS     // SC_*
	#define NORASTEROPS       // Binary and Tertiary raster ops
	#define NOSHOWWINDOW      // SW_*
	#define OEMRESOURCE       // OEM Resource values
	#define NOATOM            // Atom Manager routines
	#define NOCLIPBOARD       // Clipboard routines
	#define NOCOLOR           // Screen colors
	#define NOCTLMGR          // Control and Dialog routines
	#define NODRAWTEXT        // DrawText() and DT_*
	#define NOGDI             // All GDI defines and routines
	#define NOKERNEL          // All KERNEL defines and routines
	#define NOUSER            // All USER defines and routines
	#define NOMB              // MB_* and MessageBox()
	#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
	#define NOMETAFILE        // typedef METAFILEPICT
	#define NOMSG             // typedef MSG and associated routines
	#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
	#define NOSCROLL          // SB_* and scrolling routines
	#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
	#define NOSOUND           // Sound driver routines
	#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
	#define NOWH              // SetWindowsHook and WH_*
	#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
	#define NOCOMM            // COMM driver routines
	#define NOKANJI           // Kanji support stuff.
	#define NOHELP            // Help engine interface.
	#define NOPROFILER        // Profiler interface.
	#define NODEFERWINDOWPOS  // DeferWindowPos routines
	#define NOMCX             // Modem Configuration Extensions
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
	#include <sys/select.h>
    #include <unistd.h>
#endif

struct Connection {
	int sockfd;
	struct sockaddr_in address;
	std::string ip;
	uint16_t port;
};

enum class NetworkHostState {
	PREPARE = 0,
	TOPOLOGIZE,
	TOPOLOGIZE_RETURN,
	DISTRIBUTE,
	DISTRIBUTE_RETURN,
	TREEPREP,
	TREEPREP_RETURN,
	TREEDIST,
	TREEBUILD,
	TREEBUILD_RETURN,
	EVALUATE,
	EVALUATE_RETURN,
};

enum class NetworkClientState {
	EMPTY = 0,
	RECEIVE_PARTICLES,
	RECEIVE_TREE,
	CONSTRUCT_TREE,
	EVALUATE_PARTICLES
};

struct TreeSet {
	int size;
	Octtree trees[8];
};

class Network {
public:
	Network(Simulation* context) : m_SimulationRef(context) {}
	~Network();
public:
	void Open(std::string ip, uint16_t port);
	void Close();
public:
	void HostProcess();
	void ClientProcess();
	void SetHostState(NetworkHostState state);
	void SetClientState(NetworkClientState state);
public:
	void SendNetworkInfo();
	void VerifyConnection();
	void ReceiveID();
private:
	Simulation* m_SimulationRef = nullptr;
	Connection m_MainConnection = { 0 };
	Connection m_Neighbor = { 0 };
	size_t m_NeighborID = 0;
	size_t m_ClientID = 0;
	NetworkHostState m_HostState = NetworkHostState::PREPARE;
	NetworkClientState m_ClientState = NetworkClientState::EMPTY;
	std::vector<struct sockaddr> m_ClientAddressPoints;
private:
	size_t m_HostSliceInd = 0;
	size_t m_HostSliceSize = 0;
private:
	TreeSet m_Trees;
	std::vector<Particle> m_Seeds;
	size_t m_IgnoreThreshold;
};
