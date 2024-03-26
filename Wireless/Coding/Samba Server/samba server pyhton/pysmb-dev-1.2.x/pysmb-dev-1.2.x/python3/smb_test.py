import tempfile
from smb.SMBConnection import SMBConnection

# There will be some mechanism to capture userID, password, client_machine_name, server_name and server_ip
# client_machine_name can be an arbitary ASCII string
# server_name should match the remote machine name, or else the connection will be rejected
conn = SMBConnection(userID, password, client_machine_name, server_name, use_ntlm_v2 = True)
assert conn.connect(server_ip, 139)

file_obj = tempfile.NamedTemporaryFile()
file_attributes, filesize = conn.retrieveFile('smbtest', '/rfc1001.txt', file_obj)

# Retrieved file contents are inside file_obj
# Do what you need with the file_obj and then close it
# Note that the file obj is positioned at the end-of-file,
# so you might need to perform a file_obj.seek() if you need
# to read from the beginning
file_obj.close()