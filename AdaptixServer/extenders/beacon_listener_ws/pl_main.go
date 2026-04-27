package main

import (
	"AdaptixServer/core/extender"
	"AdaptixServer/core/utils/logs"
	"encoding/json"
	"os"
)

const (
	EXTENDER_NAME    = "WebSocket Listener"
	EXTENDER_VERSION = "1.0"
)

var (
	ExtenderInfo extender.ExtenderInfo
	Listener     *WebSocketListener
)

func main() {
	ExtenderInfo = extender.ExtenderInfo{
		Name:        EXTENDER_NAME,
		Version:     EXTENDER_VERSION,
		Description: "WebSocket-based C2 listener with AES-256-GCM encryption",
		Author:      "AdaptixC2",
	}

	configPath := os.Args[1]
	config, err := LoadConfig(configPath)
	if err != nil {
		logs.Error("", "Failed to load config: "+err.Error())
		os.Exit(1)
	}

	Listener = NewWebSocketListener(config)
	Listener.Start()
}

type WebSocketListener struct {
	config   *Config
	server   *WSServer
	sessions map[string]*AgentSession
}

func NewWebSocketListener(config *Config) *WebSocketListener {
	return &WebSocketListener{
		config:   config,
		sessions: make(map[string]*AgentSession),
	}
}

func (l *WebSocketListener) Start() {
	l.server = NewWSServer(l.config)
	l.server.OnConnect = l.handleConnect
	l.server.OnDisconnect = l.handleDisconnect
	l.server.OnMessage = l.handleMessage

	logs.Info("", "WebSocket Listener starting on "+l.config.BindAddress+":"+l.config.BindPort)
	l.server.Start()
}

func (l *WebSocketListener) handleConnect(conn *WSConnection) {
	session := NewAgentSession(conn, l.config)
	l.sessions[session.ID] = session
	logs.Info("", "New WebSocket connection: "+session.ID)
}

func (l *WebSocketListener) handleDisconnect(conn *WSConnection) {
	if session, ok := l.sessions[conn.ID]; ok {
		session.Close()
		delete(l.sessions, conn.ID)
		logs.Info("", "WebSocket disconnected: "+session.ID)
	}
}

func (l *WebSocketListener) handleMessage(conn *WSConnection, data []byte) {
	session, ok := l.sessions[conn.ID]
	if !ok {
		return
	}

	// Decrypt message
	decrypted, err := session.Decrypt(data)
	if err != nil {
		logs.Error("", "Decryption failed: "+err.Error())
		return
	}

	// Process message
	response := l.processAgentMessage(session, decrypted)

	// Encrypt and send response
	if response != nil {
		encrypted, _ := session.Encrypt(response)
		conn.Send(encrypted)
	}
}

func (l *WebSocketListener) processAgentMessage(session *AgentSession, data []byte) []byte {
	var msg AgentMessage
	if err := json.Unmarshal(data, &msg); err != nil {
		return nil
	}

	switch msg.Type {
	case "init":
		return l.handleInit(session, msg)
	case "checkin":
		return l.handleCheckin(session, msg)
	case "task_result":
		return l.handleTaskResult(session, msg)
	default:
		logs.Warn("", "Unknown message type: "+msg.Type)
	}

	return nil
}

func (l *WebSocketListener) handleInit(session *AgentSession, msg AgentMessage) []byte {
	// Initialize agent session
	session.AgentID = msg.AgentID
	session.Hostname = msg.Hostname
	session.OS = msg.OS
	session.Arch = msg.Arch
	session.User = msg.User
	session.PID = msg.PID

	// Generate session key
	session.GenerateSessionKey()

	// Send acknowledgment with session key
	response := map[string]interface{}{
		"type":       "init_ack",
		"session_id": session.ID,
		"key":        session.PublicKey,
	}

	data, _ := json.Marshal(response)
	return data
}

func (l *WebSocketListener) handleCheckin(session *AgentSession, msg AgentMessage) []byte {
	// Update last checkin time
	session.LastCheckin = msg.Timestamp

	// Get pending tasks
	tasks := l.getPendingTasks(session.AgentID)

	response := map[string]interface{}{
		"type":  "tasks",
		"tasks": tasks,
	}

	data, _ := json.Marshal(response)
	return data
}

func (l *WebSocketListener) handleTaskResult(session *AgentSession, msg AgentMessage) []byte {
	// Process task result
	taskID := msg.TaskID
	result := msg.Result

	// Store result
	l.storeTaskResult(session.AgentID, taskID, result)

	response := map[string]interface{}{
		"type":    "task_ack",
		"task_id": taskID,
	}

	data, _ := json.Marshal(response)
	return data
}

func (l *WebSocketListener) getPendingTasks(agentID string) []interface{} {
	// TODO: Get tasks from database
	return []interface{}{}
}

func (l *WebSocketListener) storeTaskResult(agentID, taskID string, result []byte) {
	// TODO: Store result in database
}
