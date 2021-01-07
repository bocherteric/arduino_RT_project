# ifndef CAN_FRAME_STREAM_H
# define CAN_FRAME_STREAM_H



class can_frame_stream {

    static const int buffsize = 10;
    can_frame cf_buffer[ buffsize ];
    int read_index; //where to read next message
    int write_index; //where to write next message
    bool write_lock; //buffer full
  public:
    can_frame_stream()
      : read_index( 0 ) , write_index( 0 ), write_lock( false )
    {};

    int put( can_frame & ); //?
    int get( can_frame & );
};

inline int can_frame_stream::put( can_frame &frame ) {
  if ( write_lock )
    return 0; //buffer full
  cf_buffer[ write_index ] = frame;
  write_index = ( ++write_index ) % buffsize;
  if ( write_index == read_index)
    write_lock = true; //cannot write more
  return 1;
}

inline int can_frame_stream::get( can_frame &frame ) {
  if ( !write_lock && ( read_index == write_index ) )
    return 0; //empty buffer
  if ( write_lock && ( read_index == write_index ) )
    write_lock = false; //release lock
  frame = cf_buffer[ read_index ];
  read_index = ( ++read_index ) % buffsize;
  return 1;
}

#endif
