package main

import (
    "log"
	"backend/server"
	"net/http"
)

func main() {
    http.HandleFunc("/", server.Handler)
    log.Fatal(http.ListenAndServe(":8080", nil))
}