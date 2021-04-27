namespace pdlfs{

namespace minicarp{


class Producer : public MiniCarp{

  Producer(const MiniCarpOptions& options)
    : MiniCarp(options);

  Status Produce(char* fname);
  
}


}//minicarp

}//pdlfs
