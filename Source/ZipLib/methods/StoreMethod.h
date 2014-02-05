#pragma once
#include "ICompressionMethod.h"
#include "../compression/store/store_encoder.h"
#include "../compression/store/store_decoder.h"

#include <memory>

class StoreMethod :
  public ICompressionMethod
{
  public:
    ZIP_METHOD_CLASS_PROLOGUE(
      StoreMethod,
      store_encoder, store_decoder,
      /* CompressionMethod */ 0,
      /* VersionNeededToExtract */ 10
    );
};

class ICompressionMethodMayBeStored
  : public ICompressionMethod
{
  public:
    typedef std::shared_ptr<ICompressionMethodMayBeStored> Ptr;

  protected:
    void SetIsStored(bool isStored = true)
    {
      // save original encoder, so we can potentionally restore them back
      if (_originalEncoder == nullptr)
      {
        _originalEncoder = this->GetEncoder();
      }

      if (isStored)
      {
        if (_storeEncoder == nullptr)
        {
          _storeEncoder = std::make_shared<store_encoder>();
        }

        this->SetEncoder(_storeEncoder);
      }
      else
      {
        this->SetEncoder(_originalEncoder);
      }
    }

  private:
    typedef std::shared_ptr<store_encoder>  store_encoder_t;
    typedef ICompressionMethod::encoder_t   encoder_t;

    encoder_t _originalEncoder;
    store_encoder_t _storeEncoder;
};