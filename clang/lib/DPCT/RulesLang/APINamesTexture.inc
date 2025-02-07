//===--------------- APINamesTexture.inc ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
CONDITIONAL_FACTORY_ENTRY(
    [](const CallExpr *C) -> bool { return C->getNumArgs(); },
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_RENAMED("cudaCreateChannelDesc",
                                          MapNames::getDpctNamespace() +
                                              "image_channel")),
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEMPLATED("cudaCreateChannelDesc",
                        TEMPLATED_CALLEE(MapNames::getDpctNamespace() +
                                             "image_channel::create",
                                         std::vector<size_t>(1, 0)))))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ENTRY_RENAMED("cudaCreateChannelDescHalf",
                                      MapNames::getDpctNamespace() +
                                          "image_channel::create<" +
                                          MapNames::getClNamespace() +
                                          "half>"))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ENTRY_RENAMED("cudaCreateChannelDescHalf1",
                                      MapNames::getDpctNamespace() +
                                          "image_channel::create<" +
                                          MapNames::getClNamespace() +
                                          "half>"))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ENTRY_RENAMED("cudaCreateChannelDescHalf2",
                                      MapNames::getDpctNamespace() +
                                          "image_channel::create<" +
                                          MapNames::getClNamespace() +
                                          "half2>"))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ENTRY_RENAMED("cudaCreateChannelDescHalf4",
                                      MapNames::getDpctNamespace() +
                                          "image_channel::create<" +
                                          MapNames::getClNamespace() +
                                          "half4>"))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
                            POINTER_CHECKER(0),
                            MEMBER_CALL_FACTORY_ENTRY("cudaUnbindTexture",
                                                      DEREF(0), true, "detach"),
                            MEMBER_CALL_FACTORY_ENTRY(
                                "cudaUnbindTexture", ARG(0), false, "detach"))))

ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("cudaDestroyTextureObject",
                       CALL(MapNames::getDpctNamespace() +
                                "experimental::destroy_bindless_image",
                            ARG(0), QUEUESTR)),
    DELETER_FACTORY_ENTRY("cudaDestroyTextureObject", ARG(0))))
ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("cuTexObjectDestroy",
                       CALL(MapNames::getDpctNamespace() +
                                "experimental::destroy_bindless_image",
                            ARG(0), QUEUESTR)),
    DELETER_FACTORY_ENTRY("cuTexObjectDestroy", ARG(0))))
ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("cudaDestroySurfaceObject",
                       CALL(MapNames::getDpctNamespace() +
                                "experimental::destroy_bindless_image",
                            ARG(0), QUEUESTR)),
    DELETER_FACTORY_ENTRY("cudaDestroySurfaceObject", ARG(0))))

ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("cuSurfObjectDestroy",
                       CALL(MapNames::getDpctNamespace() +
                                "experimental::destroy_bindless_image",
                            ARG(0), QUEUESTR)),
    DELETER_FACTORY_ENTRY("cuSurfObjectDestroy", ARG(0))))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cudaGetTextureObjectResourceDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_data", ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cudaGetTextureObjectResourceDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true, "get_data")))))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cudaGetTextureObjectTextureDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_sampling_info", ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cudaGetTextureObjectTextureDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true,
                                            "get_sampling_info")))))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cuTexObjectGetResourceDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_data", ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cuTexObjectGetResourceDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true, "get_data")))))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cuTexObjectGetTextureDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_sampling_info",
             ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cuTexObjectGetTextureDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true,
                                            "get_sampling_info")))))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cudaGetSurfaceObjectResourceDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_data", ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cudaGetSurfaceObjectResourceDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true, "get_data")))))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cuSurfObjectGetResourceDesc", DEREF(0),
        CALL(MapNames::getDpctNamespace() + "experimental::get_data", ARG(1)))),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                                "cuSurfObjectGetResourceDesc", DEREF(0),
                                MEMBER_CALL(ARG(1), true, "get_data")))))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("tex1D", 0x01,
                      MapNames::getClNamespace() +
                          "ext::oneapi::experimental::sample_image",
                      1)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("tex1D", 0x01, "read", 1)))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ENTRY_TEXTURE("tex1DLod", 0x11,
                  MapNames::getClNamespace() +
                      "ext::oneapi::experimental::sample_mipmap",
                  1),
    UNSUPPORT_FACTORY_ENTRY("tex1DLod", Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("tex1DLod"),
                            ARG("--use-experimental-features=bindless_images")))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("tex2D", 0x02,
                      MapNames::getClNamespace() +
                          "ext::oneapi::experimental::sample_image",
                      1, 2)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("tex2D", 0x02, "read", 1, 2)))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ENTRY_TEXTURE("tex2DLod", 0x12,
                  MapNames::getClNamespace() +
                      "ext::oneapi::experimental::sample_mipmap",
                  1, 2),
    UNSUPPORT_FACTORY_ENTRY("tex2DLod", Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("tex2DLod"),
                            ARG("--use-experimental-features=bindless_images")))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ENTRY_TEXTURE("tex3D", 0x03,
                  MapNames::getClNamespace() +
                      "ext::oneapi::experimental::sample_image",
                  1, 2, 3))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ENTRY_TEXTURE("tex3DLod", 0x13,
                  MapNames::getClNamespace() +
                      "ext::oneapi::experimental::sample_mipmap",
                  1, 2, 3),
    UNSUPPORT_FACTORY_ENTRY("tex3DLod", Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("tex3DLod"),
                            ARG("--use-experimental-features=bindless_images")))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        WARNING_FACTORY_ENTRY(
            "tex1Dfetch",
            ENTRY_TEXTURE("tex1Dfetch", 0x01,
                          MapNames::getClNamespace() +
                              "ext::oneapi::experimental::sample_image",
                          1),
            Diagnostics::TEX_FETCH)),
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        WARNING_FACTORY_ENTRY("tex1Dfetch",
                              ENTRY_TEXTURE("tex1Dfetch", 0x01, "read", 1),
                              Diagnostics::TEX_FETCH)))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("tex1DLayered", 0xF1,
                      MapNames::getClNamespace() +
                          "ext::oneapi::experimental::sample_image_array",
                      1)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("tex1DLayered", 0xF1, "read", 2, 1)))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    ENTRY_TEXTURE("tex2DLayered", 0xF2,
                  MapNames::getClNamespace() +
                      "ext::oneapi::experimental::sample_image_array",
                  1, 2),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("tex2DLayered", 0xF2, "read", 3, 1,
                                          2)))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ENTRY_BIND("cudaBindTexture",
                                                      1 /*start arg idx*/,
                                                      2 /*size in bytes*/)))
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ENTRY_BIND("cudaBindTexture2D", 1 /*start arg idx*/,
                                  2 /*size in dimension x*/,
                                  3 /*size in dimension y*/, 4 /*pitch*/)))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ENTRY_BIND("cudaBindTextureToArray",
                                                      0 /*start arg idx*/)))
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ENTRY_BIND("cudaBindTextureToMipmappedArray",
                                  0 /*start arg idx*/)))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudaCreateTextureObject", DEREF(0),
                            CALL(DpctGlobalInfo::useExtBindlessImages()
                                     ? MapNames::getDpctNamespace() +
                                           "experimental::create_bindless_image"
                                     : MapNames::getDpctNamespace() +
                                           "create_image_wrapper",
                                 DEREF(1), DEREF(2)))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cuTexObjectCreate", DEREF(0),
                            CALL(DpctGlobalInfo::useExtBindlessImages()
                                     ? MapNames::getDpctNamespace() +
                                           "experimental::create_bindless_image"
                                     : MapNames::getDpctNamespace() +
                                           "create_image_wrapper",
                                 DEREF(1), DEREF(2)))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cudaCreateSurfaceObject", DEREF(0),
                            CALL(DpctGlobalInfo::useExtBindlessImages()
                                     ? MapNames::getDpctNamespace() +
                                           "experimental::create_bindless_image"
                                     : MapNames::getDpctNamespace() +
                                           "create_image_wrapper",
                                 DEREF(1)))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cuSurfObjectCreate", DEREF(0),
                            CALL(DpctGlobalInfo::useExtBindlessImages()
                                     ? MapNames::getDpctNamespace() +
                                           "experimental::create_bindless_image"
                                     : MapNames::getDpctNamespace() +
                                           "create_image_wrapper",
                                 DEREF(1)))))

ENTRY_UNSUPPORTED("cudaGetTextureObjectResourceViewDesc",
                  Diagnostics::API_NOT_MIGRATED)

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cuArray3DCreate_v2", DEREF(0),
                            NEW(DpctGlobalInfo::useExtBindlessImages()
                                    ? MapNames::getDpctNamespace() +
                                          "experimental::image_mem_wrapper"
                                    : MapNames::getDpctNamespace() +
                                          "image_matrix",
                                ARG(1)))))
FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
                            "cuArrayCreate_v2", DEREF(0),
                            NEW(DpctGlobalInfo::useExtBindlessImages()
                                    ? MapNames::getDpctNamespace() +
                                          "experimental::image_mem_wrapper"
                                    : MapNames::getDpctNamespace() +
                                          "image_matrix",
                                ARG(1)))))
ASSIGNABLE_FACTORY(DELETER_FACTORY_ENTRY("cuArrayDestroy", ARG(0)))
ENTRY_UNSUPPORTED("cuTexObjectGetResourceViewDesc", Diagnostics::API_NOT_MIGRATED)
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(CONDITIONAL_FACTORY_ENTRY(
        UseExtBindlessImages,
        MEMBER_CALL_FACTORY_ENTRY("cuTexRefSetArray", ARG(0), true, "attach",
                                  ARG(1)),
        MEMBER_CALL_FACTORY_ENTRY(
            "cuTexRefSetArray", ARG(0), true, "attach",
            CALL(MapNames::getDpctNamespace() + "image_data", ARG(1))))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY("cuTexRefSetAddressMode",
                                                 ARG(0), true, "set", ARG(2))))
FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY("cuTexRefSetFilterMode",
                                                 ARG(0), true, "set", ARG(1))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cuTexRefGetAddressMode", DEREF(0),
        MEMBER_CALL(ARG(1), true, "get_addressing_mode"))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY("cuTexRefGetFilterMode", DEREF(0),
                                            MEMBER_CALL(ARG(1), true,
                                                        "get_filtering_mode"))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    ASSIGNABLE_FACTORY(ASSIGN_FACTORY_ENTRY(
        "cuTexRefGetFlags", DEREF(0),
        BO(BinaryOperatorKind::BO_Shl,
           MEMBER_CALL(ARG(1), true, "is_coordinate_normalized"), ARG("1")))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cuTexRefSetAddress_v2", ARG(1), true, "attach",
                            ARG(2), ARG(3))))

FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                        ASSIGNABLE_FACTORY(MEMBER_CALL_FACTORY_ENTRY(
                            "cuTexRefSetAddress2D_v3", ARG(0), true, "attach",
                            ARG(1), ARG(2), ARG(3))))

FEATURE_REQUEST_FACTORY(
    HelperFeatureEnum::device_ext,
    MULTI_STMTS_FACTORY_ENTRY(
        "cuTexRefSetFormat", true, false, true, true,
        MEMBER_CALL(ARG(0), true, "set_channel_type", ARG(1)),
        MEMBER_CALL(ARG(0), true, "set_channel_num", ARG(2))))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("surf1Dwrite",
                       CALL(MapNames::getClNamespace() +
                                "ext::oneapi::experimental::write_image",
                            ARG(1),
                            CALL("int", BO(BinaryOperatorKind::BO_Div, ARG(2),
                               CALL("sizeof", ARG(0)))),
                            ARG(0))),
    UNSUPPORT_FACTORY_ENTRY("surf1Dwrite",
                            Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("surf1Dwrite"),
                            ARG("--use-experimental-features=bindless_images")))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("surf2Dwrite",
                       CALL(MapNames::getClNamespace() +
                                "ext::oneapi::experimental::write_image",
                            ARG(1),
                            CALL(MapNames::getClNamespace() + "int2",
                                 BO(BinaryOperatorKind::BO_Div, ARG(2),
                                    CALL("sizeof", ARG(0))),
                                 ARG(3)),
                            ARG(0))),
    UNSUPPORT_FACTORY_ENTRY("surf2Dwrite",
                            Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("surf2Dwrite"),
                            ARG("--use-experimental-features=bindless_images")))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("surf2DLayeredwrite",
                       CALL(MapNames::getClNamespace() +
                                "ext::oneapi::experimental::write_image_array",
                            ARG(1),
                            CALL(MapNames::getClNamespace() + "int2",
                                 BO(BinaryOperatorKind::BO_Div, ARG(2),
                                    CALL("sizeof", ARG(0))),
                                 ARG(3)),
                            ARG(4), ARG(0))),
    UNSUPPORT_FACTORY_ENTRY("surf2DLayeredwrite",
                            Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("surf2DLayeredwrite"),
                            ARG("--use-experimental-features=bindless_images")))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    CALL_FACTORY_ENTRY("surf3Dwrite",
                       CALL(MapNames::getClNamespace() +
                                "ext::oneapi::experimental::write_image",
                            ARG(1),
                            CALL(MapNames::getClNamespace() + "int3",
                                 BO(BinaryOperatorKind::BO_Div, ARG(2),
                                    CALL("sizeof", ARG(0))),
                                 ARG(3), ARG(4)),
                            ARG(0))),
    UNSUPPORT_FACTORY_ENTRY("surf3Dwrite",
                            Diagnostics::TRY_EXPERIMENTAL_FEATURE,
                            ARG("surf3Dwrite"),
                            ARG("--use-experimental-features=bindless_images")))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("surf1Dread", 0x01,
                      MapNames::getLibraryHelperNamespace() +
                          "experimental::fetch_image_by_byte",
                      1)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("surf1Dread", 0x01, "read_byte", 1)))

CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("surf2Dread", 0x02,
                      MapNames::getLibraryHelperNamespace() +
                          "experimental::fetch_image_by_byte",
                      1, 2)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("surf2Dread", 0x02, "read_byte", 1,
                                          2)))
CONDITIONAL_FACTORY_ENTRY(
    UseExtBindlessImages,
    FEATURE_REQUEST_FACTORY(
        HelperFeatureEnum::device_ext,
        ENTRY_TEXTURE("surf3Dread", 0x03,
                      MapNames::getLibraryHelperNamespace() +
                          "experimental::fetch_image_by_byte",
                      1, 2, 3)),
    FEATURE_REQUEST_FACTORY(HelperFeatureEnum::device_ext,
                            ENTRY_TEXTURE("surf3Dread", 0x03, "read_byte", 1, 2,
                                          3)))
