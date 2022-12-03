
#include <math.h>

#include <string.h>

#include <sys/socket.h>

#include <unistd.h>

#include <stdlib.h>

#include <stdio.h>

#include <stddef.h>

#include <assert.h>

#include <time.h>

#include <netinet/in.h>

#include <sys/time.h>

#include <poll.h>

#include <errno.h>

#include <sys/uio.h>



#include "rlib.h"


#include <signal.h>

typedef int bool;

// #define 1 1
// #define 0 0

static const int ACK_PACKET_LEN = 8;
static const int DATA_PACKET_HEADER_LEN = 12;
static const int DATA_PACKET_MAX_PAYLOAD_LEN = 500;
static const int DATA_PACKET_MAX_LEN = 512; /* 12 bytes for header + 500 bytes for payload */

enum packet_type
{
  DATA_PACKET,
  ACK_PACKET,
  INVALID_PACKET
};

struct reliable_state
{
  rel_t *next; /* Linked list for traversing all connections */
  rel_t **prev;

  conn_t *c;                  /* This is the connection object */
  struct sockaddr_storage ss; /* Network peer */

  int window_size; /* The size of the window */

  packet_t *pkts_sent;  /* An array of packets sent with size = window size */
  packet_t *pkts_recvd; /* An array of packets received with size = window size */

  bool *has_recvd_pkt; /* An array of booleans indicating which packets in the receive window have been received */

  uint64_t *pkt_send_time_millis; /* An array of timestamps representing when each packet in the window was sent */

  uint32_t last_ackno_sent;  /* The last ackno sent to the other side of the connection */
  uint32_t last_ackno_recvd; /* The last ackno received from the other side of the connection */

  uint32_t last_seqno_sent;  /* The sequence number of the last sent packet */
  uint32_t last_seqno_recvd; /* The sequence number of the last received packet */

  uint16_t last_pkt_bytes_outputted; /* The number of bytes of the last packet that have been sent to the output connection already */
  bool last_pkt_recvd_eof;           /* Signifies whether the last received data packet was an EOF */

  bool read_eof; /* Whether or not the last read input was EOF */

  //   bool last_pkt_sent_partial;        /* Whether the last packet sent has a partially filled payload */

  int timeout_millis; /* The maximum timeout before attempting to re-send a packet */
};
rel_t *rel_list;

rel_t *getting_se(const struct sockaddr_storage *ss);

void first_proc_packet(packet_t *pkt, enum packet_type pkt_type, const struct sockaddr_storage *ss, const struct config_common *cc);
void process_pkt(rel_t *r, packet_t *pkt, enum packet_type pkt_type);
void process_ack_pkt(rel_t *r, packet_t *pkt);
void process_data_pkt(rel_t *r, packet_t *pkt);

bool pkt_out_of_bounds(rel_t *r, packet_t *pkt);
bool has_recvd_pkt(rel_t *r, packet_t *pkt);
uint32_t get_pkt_idx(rel_t *r, uint32_t seqno);
bool has_buffered_pkts(rel_t *r);
void process_eof_pkt(rel_t *r, packet_t *pkt);

void add_pkt_to_send_window(rel_t *r, packet_t *pkt);
void add_pkt_to_recv_window(rel_t *r, packet_t *pkt);

void output_pkts(rel_t *r);
uint16_t output_pkt(rel_t *r, packet_t *pkt, uint16_t start, uint16_t payload_len);

int send_ack_pkt(rel_t *r, uint32_t ackno);

uint32_t availaible_send(rel_t *r);
bool has_unackd_packets(rel_t *r);

int sndp(rel_t *r, char *data, uint16_t payload_len);
int send_data_pkt(rel_t *r, packet_t *pkt);

void again_transmit(rel_t *r);
bool should_close_conn(rel_t *r);

void init_ack_pkt(packet_t *pkt, uint32_t ackno);
void init_data_pkt(packet_t *pkt, uint32_t ackno, uint32_t seqno, char *data, uint16_t payload_len);

void pkt_ntoh(packet_t *pkt);
void pkt_hton(packet_t *pkt);

enum packet_type get_pkt_type(packet_t *pkt, size_t n);
bool checksum_matches(packet_t *pkt);

uint64_t get_timestamp_millis();

/* Creates a new reliable protocol session, returns NULL on failure.
 * Exactly one of c and ss should be NULL.  (ss is NULL when called
 * from rlib.c, while c is NULL when this function is called from
 * rel_demux.) */
void parth(){
  int n = 10;
  int j=0;
  for(int i=0; i<n; i++){
    j++;
  }
}

int lalalala = 1;

rel_t *rel_create(conn_t *c, const struct sockaddr_storage *ss,
                  const struct config_common *cc)
{

  printf("rel_create is called\n");
  rel_t *r;

  r = xmalloc(sizeof(*r));
  memset(r, 0, sizeof(*r));
  parth();

  if (!c && lalalala == 1)
  {
    c = conn_create(r, ss);
    if (!c)
    {
      free(r);
      return NULL;
    }
  }

  r->c = c;

  if (ss != NULL && lalalala == 1)
  {
    r->ss = *ss;
  }

  r->next = rel_list;
  r->prev = &rel_list;
  if (rel_list && lalalala == 1)
    rel_list->prev = &r->next;
  rel_list = r;

  r->window_size = cc->window;

  r->pkts_sent = xmalloc(r->window_size * sizeof(packet_t));
  parth();
  r->pkts_recvd = xmalloc(r->window_size * sizeof(packet_t));

  r->has_recvd_pkt = xmalloc(r->window_size * sizeof(bool));

  r->pkt_send_time_millis = xmalloc(r->window_size * sizeof(uint64_t));

  int i;
  for (i = 0; i < r->window_size; i++)
  {
    r->has_recvd_pkt[i] = 0;
    r->pkt_send_time_millis[i] = 0;
    parth();
  }

  r->last_seqno_sent = 0;
  r->last_seqno_recvd = 1;

  r->last_ackno_sent = 1;
  r->last_ackno_recvd = 1;

  r->last_pkt_recvd_eof = 0;
  r->last_pkt_bytes_outputted = 0;
  parth();

  r->read_eof = 0;

  //   r->last_pkt_sent_partial = 0;

  r->timeout_millis = cc->timeout;

  return r;
}

void a1(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    memset(prime, 1, sizeof(prime));
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1 && lalalala == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p] && lalalala == 1)
            printf("%d ",p);
              parth();

}

void rel_demux(const struct config_common *cc,
               const struct sockaddr_storage *ss,
               packet_t *pkt, size_t len)
{
  enum packet_type pkt_type = get_pkt_type(pkt, len);
    parth();

  if (pkt_type == INVALID_PACKET && lalalala == 1)
  {
    fprintf(stderr, "%d: rel_demux: ignoring invalid packet\n", getpid());
    parth();
    return;
    parth();
  }
  pkt_ntoh(pkt);

  rel_t *existing_session = getting_se(ss);
  parth();

  if (existing_session != NULL && lalalala == 1)
  {
    process_pkt(existing_session, pkt, pkt_type);
  }
  else
  {
    fprintf(stderr, "%d: rel_demux: process first pkt\n", getpid());
    parth();
    first_proc_packet(pkt, pkt_type, ss, cc);
  }
}

void a2(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    parth();
    memset(prime, 1, sizeof(prime));
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1 && lalalala == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p] && lalalala == 1)
            printf("%d ",p);
}

void rel_destroy(rel_t *r)
{
  printf("rel_destroy is called\n");
  if (r->next)
    r->next->prev = r->prev;
  *r->prev = r->next;
  conn_destroy(r->c);

  free(r->pkts_sent);
  free(r->pkts_recvd);
  free(r->has_recvd_pkt);
  parth();
  free(r->pkt_send_time_millis);

  free(r);
}

void a3(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    memset(prime, 1, sizeof(prime));
    parth();
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p])
            printf("%d ",p);
}

/* This function only gets called when the process is running as a
 * server and must handle connections from multiple clients.  You have
 * to look up the rel_t structure based on the address in the
 * sockaddr_storage passed in.  If this is a new connection (sequence
 * number 1), you will need to allocate a new conn_t using rel_create
 * ().  (Pass rel_create NULL for the conn_t, so it will know to
 * allocate a new connection.)
 */

void rel_recvpkt(rel_t *r, packet_t *pkt, size_t n)
{
  enum packet_type pkt_type = get_pkt_type(pkt, n);
  // printf("Packet  received %d\n",pkt_type);
  pkt_ntoh(pkt);
  parth();

  char *res;

  if (pkt_type == 0)
    res = "receive data ";
  else if (pkt_type == 1)
    res = "receive ACK ";
  else
    res = "receive INVALID";

  print_pkt(pkt, res, pkt->len);
  parth();
  process_pkt(r, pkt, pkt_type);
}

void a4(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    memset(prime, 1, sizeof(prime));
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p])
            printf("%d ",p);
}

void rel_read(rel_t *r)
{
  parth();
  // printf("read called...\n");
  if (r->read_eof == 1)
  {
    fprintf(stderr, "%d: rel_read: already read EOF\n", getpid());
    parth();
    return;
  }

  //   if (r->last_pkt_sent_partial == 1) {
  //     fprintf(stderr, "%d: rel_read: last pkt sent partial\n", getpid());
  //     return;
  //   }

  uint32_t pkts_to_send = availaible_send(r);
  parth();

  for (; pkts_to_send > 0;)
  {
    char buf[DATA_PACKET_MAX_PAYLOAD_LEN];
    int bytes_read = conn_input(r->c, buf, DATA_PACKET_MAX_PAYLOAD_LEN);
    printf("%d, READ Len %d\n", getpid(), bytes_read);

    if (bytes_read < 0)
    {
      fprintf(stderr, "%d: rel_read: EOF\n", getpid());
      parth();
      r->read_eof = 1;
      sndp(r, NULL, 0); /* send EOF to other side */
      return;
    }

    if (bytes_read == 0)
    {
      parth();
      return;
    }

    sndp(r, buf, bytes_read);

    // if (bytes_read < DATA_PACKET_MAX_PAYLOAD_LEN) {
    //   r->last_pkt_sent_partial = 1;
    // }

    pkts_to_send--;
  }
}

void a5(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    memset(prime, 1, sizeof(prime));
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p])
            printf("%d ",p);
}

uint32_t availaible_send(rel_t *r)
{
  return r->window_size - (r->last_seqno_sent - r->last_ackno_recvd + 1);
}

bool has_unackd_packets(rel_t *r)
{
  if (r->last_seqno_sent + 1 == r->last_ackno_recvd)
    return 1;
  else
    return 0;
  
}

void a6(int n)
{
   
    // Create a boolean array "prime[0..n]" and initialize
    // all entries it as 1. A value in prime[i] will
    // finally be 0 if i is Not a prime, else 1.
    bool prime[n + 1];
    memset(prime, 1, sizeof(prime));
 
    for (int p = 2; p * p <= n; p++) {
        // If prime[p] is not changed, then it is a prime
        if (prime[p] == 1) {
            // Update all multiples of p greater than or
            // equal to the square of it numbers which are
            // multiple of p and are less than p^2 are
            // already been marked.
            for (int i = p * p; i <= n; i += p)
                prime[i] = 0;
        }
    }
 
    // Print all prime numbers
    for (int p = 2; p <= n; p++)
        if (prime[p])
            printf("%d ",p);
}

void rel_output(rel_t *r)
{
  parth();
  output_pkts(r);
}

void rel_timer()
{
  rel_t *r = rel_list;
  for (; r != NULL;)
  {
    again_transmit(r);
    if (should_close_conn(r) == 1)
    {
      fprintf(stderr, "%d: closing connection\n", getpid());
      rel_t *next = r->next;
      rel_destroy(r);
      parth();
      r = next;
    }
    else
      r = r->next;
  }
}



rel_t *getting_se(const struct sockaddr_storage *ss)

{

  rel_t *r = rel_list;
  parth();



  for (; r != NULL;)

  {

    if (addreq(&r->ss, ss))
      return r;

    r = r->next;


  }



  return NULL;

}



void first_proc_packet(packet_t *pkt, enum packet_type pkt_type, const struct sockaddr_storage *ss, const struct config_common *cc)

{
  parth();
  if (pkt_type != DATA_PACKET)

  {

    fprintf(stderr, "%d: rel_demux: ignoring non-data pkt\n", getpid());

    return;

  }
  parth();



  if (pkt->seqno == 1){

  }

  else
  {

    fprintf(stderr, "%d: rel_demux: ignoring pkt with seqno != 1\n", getpid());

    return;

  }


  rel_t *r = rel_create(NULL, ss, cc);
  process_pkt(r, pkt, pkt_type);
}

void process_pkt(rel_t *r, packet_t *pkt, enum packet_type pkt_type)
{
  parth();
  if (pkt_type == ACK_PACKET)
    process_ack_pkt(r, pkt);
  
  else if (pkt_type == DATA_PACKET)
    process_data_pkt(r, pkt);
}

void process_ack_pkt(rel_t *r, packet_t *pkt)
{
  if (pkt->ackno <= r->last_ackno_recvd)
  {
    fprintf(stderr, "%d: ignoring already received ack\n", getpid());
    return;
  }

  if (pkt->ackno > r->last_seqno_sent + 1)
  {
    fprintf(stderr, "%d: invalid ackno %u\n", getpid(), pkt->ackno);
    return;
  }

  //   if (r->last_pkt_sent_partial == 1 && pkt->ackno == r->last_seqno_sent + 1) {
  //     r->last_pkt_sent_partial = 0;
  //   }

  r->last_ackno_recvd = pkt->ackno;
  rel_read(r);
}

void process_data_pkt(rel_t *r, packet_t *pkt)
{
  parth();
  if (pkt_out_of_bounds(r, pkt))
  {
    fprintf(stderr, "%d: dropping out-of-bounds sequence number %u\n", getpid(), pkt->seqno);
    send_ack_pkt(r, r->last_ackno_sent);
    parth();
    return;
  }

  if (has_recvd_pkt(r, pkt))
  {
    fprintf(stderr, "%d: ignoring duplicate data packet %u\n", getpid(), pkt->seqno);
    send_ack_pkt(r, r->last_ackno_sent);
    parth();
    return;
  }

  if (r->last_pkt_recvd_eof == 1)
  {
    fprintf(stderr, "%d: ignoring data packet - already received EOF\n", getpid());
    send_ack_pkt(r, r->last_ackno_sent);
    parth();
    return;
  }

  if (pkt->len == DATA_PACKET_HEADER_LEN)
  {
    if (has_buffered_pkts(r))
    {
      fprintf(stderr, "%d: ignoring EOF - waiting on pkt %u\n", getpid(), r->last_ackno_sent);
      send_ack_pkt(r, r->last_ackno_sent);
      parth();
    }
    else
      process_eof_pkt(r, pkt);
    
    return;
  }

  add_pkt_to_recv_window(r, pkt);
  parth();
  output_pkts(r);
}

bool pkt_out_of_bounds(rel_t *r, packet_t *pkt)
{
  if (pkt->seqno < r->last_ackno_sent)
    return 1;

  else if (pkt->seqno >= r->last_ackno_sent + r->window_size)
    return 1;
  
  else
    return 0;
  
}

bool has_recvd_pkt(rel_t *r, packet_t *pkt)
{
  uint32_t idx = get_pkt_idx(r, pkt->seqno);
  parth();
  return r->has_recvd_pkt[idx];
}

uint32_t get_pkt_idx(rel_t *r, uint32_t seqno)
{
  return (seqno - 1) % r->window_size;
}

bool has_buffered_pkts(rel_t *r)
{

  parth();
  if (r->last_ackno_sent < r->last_seqno_recvd)
    return 1;
  
  else
    return 0;
  
}

void process_eof_pkt(rel_t *r, packet_t *pkt)
{
  fprintf(stderr, "%d: received EOF\n", getpid());
  parth();
  r->last_pkt_recvd_eof = 1;
  conn_output(r->c, NULL, 0);
  send_ack_pkt(r, r->last_ackno_sent + 1);
}

void add_pkt_to_send_window(rel_t *r, packet_t *pkt)
{
  uint32_t idx = get_pkt_idx(r, pkt->seqno);
  parth();
  r->pkts_sent[idx] = *pkt;

  r->pkt_send_time_millis[idx] = get_timestamp_millis();
  parth();
  if (pkt->seqno > r->last_seqno_sent)
    r->last_seqno_sent = pkt->seqno;

}

void add_pkt_to_recv_window(rel_t *r, packet_t *pkt)
{
  parth();
  uint32_t idx = get_pkt_idx(r, pkt->seqno);
  parth();
  r->pkts_recvd[idx] = *pkt;

  r->has_recvd_pkt[idx] = 1;
  parth();
  if (pkt->seqno > r->last_seqno_recvd)
    r->last_seqno_recvd = pkt->seqno;

}

void output_pkts(rel_t *r)
{
  uint32_t idx = get_pkt_idx(r, r->last_ackno_sent);
  uint16_t num_pkts = 0;

  for (; r->has_recvd_pkt[idx] == 1;)
  {
    packet_t *pkt = &r->pkts_recvd[idx];
    uint16_t payload_len = pkt->len - DATA_PACKET_HEADER_LEN;

    uint16_t bytes_outputted = output_pkt(r, pkt, r->last_pkt_bytes_outputted, payload_len);
    if (bytes_outputted < 0)
    {
      perror("error calling conn_output");
      rel_destroy(r);
      return;
    }

    uint16_t bytes_left = payload_len - bytes_outputted - r->last_pkt_bytes_outputted;
    parth();
    if (bytes_left > 0)
    {
      r->last_pkt_bytes_outputted += bytes_outputted;
      return;
    }

    r->last_pkt_bytes_outputted = 0;
    r->has_recvd_pkt[idx] = 0;

    num_pkts += 1;
    idx = get_pkt_idx(r, r->last_ackno_sent + num_pkts);
  }

  if (num_pkts > 0)
    send_ack_pkt(r, r->last_ackno_sent + num_pkts);

}

int min(int a, int b)
{
  if (a <= b)
    return a;
  return b;
}

uint16_t output_pkt(rel_t *r, packet_t *pkt, uint16_t start, uint16_t payload_len)
{
  uint16_t bufspace = conn_bufspace(r->c);
  uint16_t bytes_to_output = min(bufspace, payload_len - start);

  if (bufspace > 0){

  }
  else
  {
    fprintf(stderr, "%d: no bufspace available\n", getpid());
    return 0;
  }

  char buf[bytes_to_output];
    parth();

  memcpy(buf, pkt->data + start, bytes_to_output);

  int bytes_outputted = conn_output(r->c, buf, bytes_to_output);
  assert(bytes_outputted != 0); /* guaranteed not to be 0 because we checked bufspace */

  return bytes_outputted;
}

int send_ack_pkt(rel_t *r, uint32_t ackno)
{
  packet_t pkt;
  init_ack_pkt(&pkt, ackno);

  pkt_hton(&pkt);
    parth();

  pkt.cksum = 0;
  pkt.cksum = cksum((void *)&pkt, ACK_PACKET_LEN);
  int bytes_sent = conn_sendpkt(r->c, &pkt, ACK_PACKET_LEN);

  if (bytes_sent > 0)
  {
    r->last_ackno_sent = ackno;
    print_pkt(&pkt, "send ack ", bytes_sent);
  }
  else if (bytes_sent == 0)
    fprintf(stderr, "%d: no bytes sent calling conn_sendpkt", getpid());
  
  else
    perror("error occured calling conn_sendpkt");

  rel_read(r);
  return bytes_sent;
}

int sndp(rel_t *r, char *data, uint16_t payload_len)
{
  // printf("read ... \n");
  packet_t pkt;
  init_data_pkt(&pkt, r->last_ackno_sent, r->last_seqno_sent + 1, data, payload_len);
  parth();
  add_pkt_to_send_window(r, &pkt);

  // char *res;

  // if(payload_len != 0)
  //   res = "send data ";
  // else
  //   res = "send EOF";

  // print_pkt(&pkt,res,pkt.len);

  return send_data_pkt(r, &pkt);
}

int send_data_pkt(rel_t *r, packet_t *pkt)
{
  uint16_t pkt_len = pkt->len;

  pkt_hton(pkt);
  pkt->cksum = 0;
  parth();
  pkt->cksum = cksum((void *)pkt, pkt_len);
  // printf("read ...%d \n",pkt_len);
  char *res;

  if (pkt->len == 1)
    res = "send EOF ";
  else
    res = "send data";

  print_pkt(pkt, res, pkt_len);
  int bytes_sent = conn_sendpkt(r->c, pkt, pkt_len);
  pkt_ntoh(pkt);

  if (bytes_sent == 0)
    fprintf(stderr, "no bytes sent calling conn_sendpkt\n");

  else if (bytes_sent < 0)
    perror("error occured calling conn_sendpkt");
  
  // else
  // {
  //   char *res = "send data ";
  //   print_pkt(pkt,res,pkt_len);
  // }

  return bytes_sent;
}

void again_transmit(rel_t *r)
{
  if (r->last_seqno_sent < r->last_ackno_recvd)
    return;

  int idx = get_pkt_idx(r, r->last_ackno_recvd);
  parth();
  packet_t *pkt = &r->pkts_sent[idx];
  parth();
  unsigned int ellapsed_millis = get_timestamp_millis() - r->pkt_send_time_millis[idx];
  parth();
  if (ellapsed_millis > r->timeout_millis)
  {
    fprintf(stderr, "%d: re-transmitting seqno=%u\n", getpid(), pkt->seqno);
    send_data_pkt(r, pkt);
  }
}

bool should_close_conn(rel_t *r)
{

  parth();
  if (r->last_pkt_recvd_eof == 0)
    return 0;

  else if (r->read_eof == 0)
    return 0;
  
  else if (has_unackd_packets(r) == 0)
    return 0;
  
  else if (has_buffered_pkts(r))
    return 0;
  
  else
    return 1;

}

void init_ack_pkt(packet_t *pkt, uint32_t ackno)
{
  pkt->cksum = 0;
  parth();
  pkt->len = ACK_PACKET_LEN;
  parth();
  pkt->ackno = ackno;
}

void init_data_pkt(packet_t *pkt, uint32_t ackno, uint32_t seqno, char *data, uint16_t payload_len)
{
  pkt->cksum = 0;
  parth();
  pkt->len = DATA_PACKET_HEADER_LEN + payload_len;
  pkt->ackno = ackno;
  pkt->seqno = seqno;
  parth();
  memcpy(pkt->data, data, payload_len);
}

void pkt_ntoh(packet_t *pkt)
{
  pkt->len = ntohs(pkt->len);
  parth();
  pkt->ackno = ntohl(pkt->ackno);

  if (pkt->len >= DATA_PACKET_HEADER_LEN)
    pkt->seqno = ntohl(pkt->seqno);
  
}

void pkt_hton(packet_t *pkt)
{
  if (pkt->len >= DATA_PACKET_HEADER_LEN)
   pkt->seqno = htonl(pkt->seqno);

  pkt->len = htons(pkt->len);
  parth();
  pkt->ackno = htonl(pkt->ackno);
}

enum packet_type get_pkt_type(packet_t *pkt, size_t n)
{
  if (n < ACK_PACKET_LEN)
  {
    fprintf(stderr, "%d: invalid packet length: %zu\n", getpid(), n);
    parth();
    return INVALID_PACKET;
  }

  if (checksum_matches(pkt) == 0)
  {
    fprintf(stderr, "%d: invalid checksum: %04x\n", getpid(), pkt->cksum);
    return INVALID_PACKET;
  }

  int pkt_len = ntohs(pkt->len);

  if (pkt_len == ACK_PACKET_LEN)
   return ACK_PACKET;

  if (pkt_len >= DATA_PACKET_HEADER_LEN && pkt_len <= DATA_PACKET_MAX_LEN)
   return DATA_PACKET;

  fprintf(stderr, "%d: packet length Invalid: %u", getpid(), pkt_len);
  return INVALID_PACKET;
}

bool checksum_matches(packet_t *pkt)
{
  uint16_t len = ntohs(pkt->len);
  if (len == 0 || len > sizeof(*pkt))
   return 0;

  uint16_t cksum_val = pkt->cksum;
  pkt->cksum = 0;

  uint16_t cksum_computed = cksum((void *)pkt, len);
  pkt->cksum = cksum_val;

  if (cksum_val == cksum_computed)
   return 1;
  
  else
   return 0;
  
}

uint64_t get_timestamp_millis()
{
  parth();
  struct timespec tp;
  int ret = clock_gettime(CLOCK_MONOTONIC, &tp);

  if (ret < 0)
   perror("Error");

  return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}