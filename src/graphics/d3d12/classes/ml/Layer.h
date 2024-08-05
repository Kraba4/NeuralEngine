#pragma once
#include <graphics/d3d12/CommonGraphicsHeaders.h>
#include <graphics/d3d12/classes/resource/BufferAndTexture.h>

#include <DirectML.h>
#include <DirectMLX.h>

#include <cstdint>
#include <vector>
#include <array>

namespace neural::graphics {
class Layer {
public:
    enum class TensorLayout
    {
        Default,
        NHWC
    };

    static Layer createIdentityLayer(IDMLDevice* a_dmlDevice,
                                     DML_TENSOR_DATA_TYPE a_dataType,
                                     const std::array<uint32_t, 4>& a_inputSizes)
    {
        Layer identityLayer;

        // Create Identity Compiled Operator
        {
            // Why are here inputStrides, see explanation in createConvolutionLayer
            std::array<uint32_t, 4> inputStrides;

            DML_BUFFER_TENSOR_DESC inputBufferTensorDesc  = getBufferTensorDesc(a_dataType, a_inputSizes, inputStrides);
            DML_TENSOR_DESC tensorDesc = { DML_TENSOR_TYPE_BUFFER, &inputBufferTensorDesc };

            DML_ELEMENT_WISE_IDENTITY_OPERATOR_DESC identityDesc{};
            identityDesc.InputTensor  = &tensorDesc;
            identityDesc.OutputTensor = &tensorDesc; // Input and output tensors have same size/type.

            DML_OPERATOR_DESC operatorDesc = { DML_OPERATOR_ELEMENT_WISE_IDENTITY, &identityDesc };
            ComPtr<IDMLOperator> op;
            DX_CALL(a_dmlDevice->CreateOperator(&operatorDesc, IID_PPV_ARGS(op.ReleaseAndGetAddressOf())));

            DML_EXECUTION_FLAGS flag = DML_EXECUTION_FLAG_NONE;
            if (a_dataType == DML_TENSOR_DATA_TYPE_FLOAT16) { 
                flag = DML_EXECUTION_FLAG_ALLOW_HALF_PRECISION_COMPUTATION;
            }                   
            DX_CALL(a_dmlDevice->CompileOperator(op.Get(), flag, IID_PPV_ARGS(&identityLayer.m_compiledOperator)));
        }
    }
    static Layer createConvolutionLayer(IDMLDevice* a_dmlDevice,
                                        DML_TENSOR_DATA_TYPE a_dataType, 
                                        const std::array<uint32_t, 4>& a_inputSizes,
                                        const std::array<uint32_t, 4>& a_filterSizes,
                                        bool a_useBiasAndActivation) 
    {
        Layer convolutionLayer;

        // Create Convolution Compiled Operator
        {
            const std::array<uint32_t, 4>& inputSizes  = a_inputSizes;
            const std::array<uint32_t, 4>& weightSizes = a_filterSizes;
            const std::array<uint32_t, 4>  biasSizes   = { 1, a_filterSizes[0], 1, 1 };
            const std::array<uint32_t, 4>  outputSizes = { a_inputSizes[0],
                                                           a_filterSizes[0],  // from filterSizes
                                                           a_inputSizes[2],
                                                           a_inputSizes[3]};

            // Strides are placed here (not just in getBufferTensorDesc) 
            // because DML_BUFFER_TENSOR_DESC refers to them by pointer
            std::array<uint32_t, 4> inputStrides;
            std::array<uint32_t, 4> weightStrides;
            std::array<uint32_t, 4> biasStrides;
            std::array<uint32_t, 4> outputStrides;

            DML_BUFFER_TENSOR_DESC inputBufferTensorDesc  = getBufferTensorDesc(a_dataType, inputSizes,  inputStrides);
            DML_BUFFER_TENSOR_DESC weightBufferTensorDesc = getBufferTensorDesc(a_dataType, weightSizes, weightStrides);
            DML_BUFFER_TENSOR_DESC biasBufferTensorDesc   = getBufferTensorDesc(a_dataType, biasSizes,   biasStrides);
            DML_BUFFER_TENSOR_DESC outputBufferTensorDesc = getBufferTensorDesc(a_dataType, outputSizes, outputStrides);

            DML_TENSOR_DESC inputTensorDesc  = { DML_TENSOR_TYPE_BUFFER, &inputBufferTensorDesc };
            DML_TENSOR_DESC weightTensorDesc = { DML_TENSOR_TYPE_BUFFER, &weightBufferTensorDesc };
            DML_TENSOR_DESC biasTensorDesc   = { DML_TENSOR_TYPE_BUFFER, &biasBufferTensorDesc };
            DML_TENSOR_DESC outputTensorDesc = { DML_TENSOR_TYPE_BUFFER, &outputBufferTensorDesc };


            // From https://github.com/microsoft/DirectML/tree/master/Samples/DirectMLSuperResolution
            // Describe, create, and compile convolution operator

            // The output size of a convolution operation is given by:
            //  height = (inputHeight - filterHeight + 2*paddingHeight) / filterStride + 1
            //  width  = (inputWidth  - filterWidth  + 2*paddingWidth ) / filterStride + 1
            //
            // We want to preserve the height and width, so assuming stride is 1, we get:
            //  paddingHeight = (filterHeight - 1) / 2
            //  paddingWidth  = (filterWidth  - 1) / 2
            // If padding is fractional, we pad unevenly with ceil/floor.
            UINT paddingHeightTop = static_cast<UINT>(ceil((a_filterSizes[2] - 1) / 2.0f));
            UINT paddingHeightBottom = static_cast<UINT>(floor((a_filterSizes[2] - 1) / 2.0f));
            UINT paddingWidthLeft = static_cast<UINT>(ceil((a_filterSizes[3] - 1) / 2.0f));
            UINT paddingWidthRight = static_cast<UINT>(floor((a_filterSizes[3] - 1) / 2.0f));

            UINT strides[] = { 1, 1 };
            UINT dilations[] = { 1, 1 };
            UINT startPadding[] = { paddingHeightTop, paddingWidthLeft };
            UINT endPadding[] = { paddingHeightBottom, paddingWidthRight };
            UINT outputPadding[] = { 0, 0 };

            DML_ACTIVATION_RELU_OPERATOR_DESC fusedReluDesc = { 0 };
            DML_OPERATOR_DESC activationDesc = { DML_OPERATOR_ACTIVATION_RELU, &fusedReluDesc };

            DML_CONVOLUTION_OPERATOR_DESC convolutionDesc = {
                .InputTensor = &inputTensorDesc,
                .FilterTensor = &weightTensorDesc,
                .BiasTensor = a_useBiasAndActivation ? &biasTensorDesc : nullptr,
                .OutputTensor = &outputTensorDesc,
                .Mode = DML_CONVOLUTION_MODE_CROSS_CORRELATION,  // TODO: what is it?
                .Direction = DML_CONVOLUTION_DIRECTION_FORWARD,
                .DimensionCount = 2,
                .Strides = strides,
                .Dilations = dilations,
                .StartPadding = startPadding,
                .EndPadding = endPadding,
                .OutputPadding = outputPadding,
                .GroupCount = 1,  // TODO: what is it?
                .FusedActivation = a_useBiasAndActivation ? &activationDesc : nullptr
            };
            DML_OPERATOR_DESC operatorDesc = { DML_OPERATOR_CONVOLUTION, &convolutionDesc };
            ComPtr<IDMLOperator> op;
            DX_CALL(a_dmlDevice->CreateOperator(&operatorDesc, IID_PPV_ARGS(op.ReleaseAndGetAddressOf())));

            DML_EXECUTION_FLAGS flag = DML_EXECUTION_FLAG_NONE;
            if (a_dataType == DML_TENSOR_DATA_TYPE_FLOAT16) { 
                flag = DML_EXECUTION_FLAG_ALLOW_HALF_PRECISION_COMPUTATION;
            }                   
            DX_CALL(a_dmlDevice->CompileOperator(op.Get(), flag, IID_PPV_ARGS(&convolutionLayer.m_compiledOperator)));
        }
    }
    
private:
    // TODO: figure out what DML_TENSOR_FLAG_OWNED_BY_DML is (can add via .Flags)
    static DML_BUFFER_TENSOR_DESC getBufferTensorDesc(DML_TENSOR_DATA_TYPE a_dataType,
                                                      const std::array<uint32_t, 4>& a_sizes,
                                                      std::array<uint32_t, 4>& a_strides) {
        getStrides(a_sizes, TensorLayout::Default, a_strides);
        uint64_t totalSize = DMLCalcBufferTensorSize(a_dataType, a_sizes.size(), a_sizes.data(), a_strides.data());

        DML_BUFFER_TENSOR_DESC bufferTensorDesc = {
            .DataType = a_dataType,
            .Flags = DML_TENSOR_FLAG_NONE,
            .DimensionCount = static_cast<UINT>(a_sizes.size()),
            .Sizes = a_sizes.data(),
            .Strides = a_strides.data(),
            .TotalTensorSizeInBytes = totalSize,
            .GuaranteedBaseOffsetAlignment = 0
        };
        return bufferTensorDesc;
    }

    static void getStrides(const std::array<uint32_t, 4>& a_sizes, TensorLayout layout, std::array<uint32_t, 4>& a_strides)
    {
        switch (layout)
        {
        case TensorLayout::NHWC:
            a_strides[0] = a_sizes[1] * a_sizes[2] * a_sizes[3];
            a_strides[1] = 1;
            a_strides[2] = a_sizes[1] * a_sizes[3];
            a_strides[3] = a_sizes[1];
            break;

        default:
            a_strides[0] = a_sizes[1] * a_sizes[2] * a_sizes[3];
            a_strides[1] = a_sizes[2] * a_sizes[3];
            a_strides[2] = a_sizes[3];
            a_strides[3] = 1;
        }
    }

    ComPtr<IDMLCompiledOperator> m_compiledOperator;
    Buffer                       m_weights;
};
}