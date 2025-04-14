#include "H264WebSocketReceiver.h"
static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
    GMainLoop* loop = static_cast<GMainLoop*>(data);

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        g_print(" End of stream\n");
        g_main_loop_quit(loop);
        break;

    case GST_MESSAGE_ERROR: {
        GError* err = nullptr;
        gchar* debug = nullptr;
        gst_message_parse_error(msg, &err, &debug);
        g_printerr(" Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        g_main_loop_quit(loop);
        break;
    }

    default:
        break;
    }
    return TRUE;
}
int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    // GStreamer 파이프라인
    GstElement* pipeline = gst_parse_launch(
        //"appsrc name=mysrc is-live=true format=3 do-timestamp=true caps=video/quicktime ! qtdemux ! h264parse ! avdec_h264 ! videoconvert ! autovideosink",
        //"appsrc name=mysrc is-live=true format=3 do-timestamp=true caps=video/quicktime ! filesink location=wsstest.mp4",
        //"appsrc name=mysrc is-live=true format=3 do-timestamp=true ! filesink location=test.mp4",
        "appsrc name=mysrc caps=video/quicktime format=3 is-live=true ! qtdemux ! decodebin ! autovideosink",
        nullptr);

    GstElement* appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "mysrc");

    // 메시지 루프 생성
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);

    // Bus 메시지 핸들링 연결
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    // 파이프라인 시작
    gst_element_set_state(pipeline, GST_STATE_PLAYING);


    // WebSocket 수신 시작 "wss://yourserver/stream"
    H264WebSocketReceiver receiver(argv[1], appsrc);
    receiver.init();
	receiver.auth();
    receiver.receive();

    // 메인 루프 실행
    g_main_loop_run(loop);

    // 종료 처리
    receiver.stop();
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}
