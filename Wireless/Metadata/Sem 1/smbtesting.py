import tempfile
import smb
from smb.SMBConnection import SMBConnection as conn

userID = "wawako"
password = "mimisijui"
client_machine_name = "wwk"
server_name = "wako server"
server_ip = "127.0.0.1"


conn1 = conn(userID, password, client_machine_name, server_name, use_ntlm_v2 = True)
assert conn1.connect(server_ip, 443)

print(conn1)
file_obj = tempfile.NamedTemporaryFile()
file_attributes, filesize = conn.retrieveFile('smbtest', '/rfc1001.txt', file_obj)


#file_obj.close()