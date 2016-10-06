$INCLUDE[header.inc]$
<div class="panel">
<div class="panel-heading">System</div>
<div class="panel-body"><label>Chip ID: </label><label class="text-info">$CHIP_ID$</label><BR>
<label>CPU Frequency: </label><label class="text-info">$CPU_FREQ$ MHz</label><BR>
<label>Free Memory: </label><label class="text-info">$FREE_MEM$ bytes</label><BR>
<label>SDK Version: </label><label class="text-info">$SDK_VER$</label><BR>
<DIV style ="$HOSTNAME_VISIBLE$"><label>Hostname: </label><label class="text-info">$HOSTNAME$</label><BR></DIV>
<DIV style ="$MDNS_VISIBLE$;"><label>mDNS name: </label><label class="text-info">$MDNS_NAME$</label><BR></DIV>
<DIV style ="$SSDP_VISIBLE$;"><label>SSDP Protocol: </label><label class="text-info">$SSDP_STATUS$</label><BR></DIV>
<DIV style ="$CAPTIVE_PORTAL_VISIBLE$;"><label>Captive Portal: </label><label class="text-info">$CAPTIVE_PORTAL_STATUS$</label><BR></DIV>
<label>Network: </label><label class="text-info">$NET_PHY$</label><BR>
<label>Sleep mode: </label><label class="text-info">$SLEEP_MODE$</label><BR>
<label>Boot version: </label><label class="text-info">$BOOT_VER$</label><BR>
<label>Baud rate: </label><label class="text-info">$BAUD_RATE$</label><BR>
<label>Web port:</label><label class="text-info">$WEB_PORT$</label><BR>
<label>Data port:</label><label class="text-info">$DATA_PORT$</label><BR>
</div></div>
<div class="panel"><div class="panel-heading">Access Point ($AP_STATUS_ENABLED$)</div>
<div class="panel-body"><label>Mac address: </label><label class="text-info">$AP_MAC$</label><BR>
<div style="$AP_VISIBILITY$;">
<label>SSID: </label><label class="text-info">$AP_SSID$</label><BR>
<label>Visible: </label><label class="text-info">$AP_IS_VISIBLE$</label><BR>
<label>Channel: </label><label class="text-info">$AP_CHANNEL$</label><BR>
<label>Authentification: </label><label class="text-info">$AP_AUTH$</label><BR>
<label>Maximum connections: </label><label class="text-info">$AP_MAX_CON$</label><BR>
<label>DHCP Server: </label><label class="text-info">$AP_DHCP_STATUS$</label><BR>
<label>IP: </label><label class="text-info">$AP_IP$</label><BR>
<label>Gateway: </label><label class="text-info">$AP_GW$</label><BR>
<label>Subnet: </label><label class="text-info">$AP_SUBNET$</label><BR>
<table class="table table-bordered table-striped">
<caption>$CONNECTED_STATIONS_NB_ITEMS$ connected station(s)</caption>
<thead><tr><th>#</th><th>Mac</th><th>IP</th></tr></thead>
<tbody>$CONNECTED_STATIONS[<TR><th>#$ROW_NUMBER$</th><td>$MAC_CONNECTED$</td><td>$IP_CONNECTED$</td></TR>]$</tbody>
</table>
</div>
</div>
</div>
<div class="panel">
<div class="panel-heading">Station ($STA_STATUS_ENABLED$)</div>
<div class="panel-body"><label>Mac address: </label><label class="text-info">$STA_MAC$</label><BR>
<div style="$STA_VISIBILITY$;">
<label>Connection to: </label><label class="text-info">$STA_SSID$</label><BR>
<label>Channel: </label><label class="text-info">$STA_CHANNEL$</label><BR>
<label>Status: </label><label class="text-info">$STA_STATUS$</label><BR>
<label>Signal strength: </label><label class="text-info">$STA_SIGNAL$%</label><BR>
<label>DHCP Client: </label><label class="text-info">$STA_DHCP_STATUS$</label><BR>
<label>IP: </label><label class="text-info">$STA_IP$</label><BR>
<label>Gateway: </label><label class="text-info">$STA_GW$</label><BR>
<label>Subnet: </label><label class="text-info">$STA_SUBNET$</label><BR>
</div>
</div>
</div>
$INCLUDE[footer.inc]$

