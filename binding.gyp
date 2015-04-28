{
  "targets": [
    {
      "target_name": "wii",
      "sources": [
      	"src/base.cc",
      	"src/wiimote.cc"
      ],
      "libraries": [
      	"-lcwiid",
      	"-lbluetooth"
      ]
    }
  ]
}
