# esp32-usb-serial
Arduino Library to use USB as OTG on ESP32 capable devices based on espressif IDF components

The code use the following versions of the components:
```
## IDF Component Manager Manifest File
dependencies:
  usb_host_ch34x_vcp: "^2"
  usb_host_cp210x_vcp: "^2"
  usb_host_ftdi_vcp: "^2"
  usb_host_vcp: "^1"
  idf: ">=5.1.0"
```

It support the following USB to Serial chips:
- CH34X
- CP210x
- FTDI FT23x 

The library is based on the IDF USB Host VCP component and it is a wrapper to use it on Arduino IDE.

Tested on ESP32S3, but should work on S2 based devices.

The sample code is a simple USB<->Serial bridge, it reads from the USB and writes to the Serial and viceversa.

I need IDF 5.1 to use the USB Host VCP component, so I had to use the latest version of the arduino-esp32 core, which is 3.0.0-alpha3 at the time of writing this.

> [!IMPORTANT]  
>
>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
