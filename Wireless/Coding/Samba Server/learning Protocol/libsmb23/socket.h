include <C:\Users\Hp\OneDrive\Desktop\PT\Samba Server\Arduino\libsmb2\socket.h>


static int
smb2_connect_async_next_addr(struct smb2_context *smb2, const struct addrinfo *base);

void
smb2_close_connecting_fds(struct smb2_context *smb2);

static int
smb2_get_credit_charge(struct smb2_context *smb2, struct smb2_pdu *pdu)

int
smb2_which_events(struct smb2_context *smb2);

t_socket
 smb2_get_fd(struct smb2_context *smb2);

const t_socket *
smb2_get_fds(struct smb2_context *smb2, size_t *fd_count, int *timeout);

static int
smb2_write_to_socket(struct smb2_context *smb2);



typedef ssize_t (*read_func)(struct smb2_context *smb2,const struct iovec *iov, int iovcnt);







