#!/bin/sh


#  frame #0: 0x00007fff97fe8f06 libsystem_kernel.dylib`__pthread_kill + 10
#  frame #1: 0x00007fff8d5674ec libsystem_pthread.dylib`pthread_kill + 90
#  frame #2: 0x00007fff93a5e6df libsystem_c.dylib`abort + 129
#  frame #3: 0x00007fff8c076c11 libc++abi.dylib`abort_message + 257
#  frame #4: 0x00007fff8c09cdff libc++abi.dylib`default_terminate_handler() + 243
#  frame #5: 0x00007fff894266c3 libobjc.A.dylib`_objc_terminate() + 124
#  frame #6: 0x00007fff8c09a00e libc++abi.dylib`std::__terminate(void (*)()) + 8
#  frame #7: 0x00007fff8c099a7a libc++abi.dylib`__cxa_throw + 121
#  frame #8: 0x00007fff88e32781 libc++.1.dylib`std::__1::__vector_base_common<true>::__throw_out_of_range() const + 71
#  frame #9: 0x0000000100009543 extractor`std::__1::vector<unsigned char, std::__1::allocator<unsigned char> >::at(unsigned long) const + 67
#  frame #10: 0x000000010000d09e extractor`BitStream::read8Bits() + 30
#  frame #11: 0x000000010000df7f extractor`BitStream::readZeroTerminatedString(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >&) + 383
#  frame #12: 0x000000010003299a extractor`ItemInfoEntry::parseBox(BitStream&) + 634
#  frame #13: 0x0000000100032684 extractor`ItemInfoBox::parseBox(BitStream&) + 180
#  frame #14: 0x000000010005b633 extractor`MetaBox::parseBox(BitStream&) + 547
#  frame #15: 0x000000010009a958 extractor`HevcImageFileReader::readStream() + 12200
#  frame #16: 0x000000010009b2c7 extractor`HevcImageFileReader::initialize(std::__1::basic_istream<char, std::__1::char_traits<char> >&) + 39
#  frame #17: 0x00000001001691ef extractor`main + 447
#  frame #18: 0x00007fff968205ad libdyld.dylib`start + 1

./rebuild_heif.sh 
./build_extractor.sh
./build_pipe.sh $1