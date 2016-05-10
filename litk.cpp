#include <cstdio>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImportImageFilter.h>
#include <itkImage.h>
#include <stdlib.h>
#include <luaT.h>
#include <lua.hpp>
#include <TH.h>

typedef itk::Image< float, 3 > ImageType;

static int itk_read3(lua_State *L) {

  const char *filename = lua_tostring(L, 1);

  typedef typename itk::ImageFileReader< ImageType > ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( filename  );

  //read image
  try
  {
    reader->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << err << std::endl;
    // TODO  make it a lua error
    abort();
  }

  THFloatTensor *sp   = THFloatTensor_newWithSize1d(3);
  THFloatTensor *orig = THFloatTensor_newWithSize1d(3);
  float *dsp = THFloatTensor_data(sp);
  float *dorig = THFloatTensor_data(orig);

  typename ImageType::PointType origin = reader->GetOutput()->GetOrigin();
  typename ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
  dorig[0] = origin[2];
  dorig[1] = origin[1];
  dorig[2] = origin[0];
  dsp[0] = spacing[2];
  dsp[1] = spacing[1];
  dsp[2] = spacing[0];

  typename ImageType::SizeType size = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
  THFloatTensor *out = THFloatTensor_newWithSize3d(size[2], size[1], size[0]);

  float *dout = THFloatTensor_data(out);
  memcpy(dout, reader->GetOutput()->GetBufferPointer(), size[0]*size[1]*size[2]*sizeof(float));

  lua_newtable(L);
  lua_pushstring(L, "spacing");
  luaT_pushudata(L, sp, "torch.FloatTensor");
  lua_settable(L, -3);

  lua_pushstring(L, "origin");
  luaT_pushudata(L, orig, "torch.FloatTensor");
  lua_settable(L, -3);

  lua_pushstring(L, "data");
  luaT_pushudata(L, out, "torch.FloatTensor");
  lua_settable(L, -3);

  return 1;
}

static int itk_write3(lua_State *L) {

  THFloatTensor *tsp = (THFloatTensor *) luaT_getfieldcheckudata(L, 1, "spacing", "torch.FloatTensor");
  THFloatTensor *torig = (THFloatTensor *) luaT_getfieldcheckudata(L, 1, "origin", "torch.FloatTensor");
  THFloatTensor *tvol = (THFloatTensor *) luaT_getfieldcheckudata(L, 1, "data", "torch.FloatTensor");
  const char *filename = lua_tostring(L, 2);

  float *sp = THFloatTensor_data(tsp);
  float *orig = THFloatTensor_data(torig);
  float *vol = THFloatTensor_data(tvol);

  typename ImageType::SizeType size;
  size[0] = tvol->size[2];
  size[1] = tvol->size[1];
  size[2] = tvol->size[0];

  typename ImageType::SpacingType spacing;
  spacing[0] = sp[2];
  spacing[1] = sp[1];
  spacing[2] = sp[0];

  typename ImageType::PointType origin;
  origin[0] = orig[2];
  origin[1] = orig[1];
  origin[2] = orig[0];

  typedef typename itk::ImportImageFilter< float, 3 > ImportFilterType;
  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
  importFilter->SetRegion(typename ImageType::RegionType(size));
  importFilter->SetSpacing(spacing);
  importFilter->SetOrigin(origin);
  int numberOfPixels = size[0] * size[1] * size[2];
  const bool importImageFilterWillOwnTheBuffer = false;
  importFilter->SetImportPointer(vol, numberOfPixels, importImageFilterWillOwnTheBuffer);

  typedef typename itk::ImageFileWriter< ImageType >  WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( filename );
  writer->SetInput(importFilter->GetOutput());
  try
  {
    writer->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << err << std::endl;
    abort();
  }
  return 0;
}


static const luaL_Reg functions[] = {
    {"read3", itk_read3},
    {"write3", itk_write3},
    {NULL, NULL},
};

extern "C" int luaopen_liblitk(lua_State *L)
{
    luaL_register(L, "litk", functions);
    return 0;
}


