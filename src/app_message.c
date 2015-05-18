#include <pebble.h>

Window *window;	
static TextLayer *s_text_layer;

char message_received[8];

// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

// Write message to buffer & send
void send_message(char* message){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	//dict_write_uint8(iter, STATUS_KEY, 0x1);
	 Tuplet value = TupletCString(STATUS_KEY, message);
   dict_write_tuplet(iter, &value);
	//dict_write_end(iter);
  	app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
    strncpy (message_received,  dict_find(received, STATUS_KEY)->value->cstring, sizeof(message_received) - 1);
    text_layer_set_text(s_text_layer, message_received);
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}
 
 static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up pressed!");
  send_message("UP");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select pressed!");
  
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down pressed!");
  send_message("DOWN");
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_text_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 5, window_bounds.size.h));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_text_layer, "Tivo Controller: press UP/Down to change Channel");
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_text_layer);
}

const uint32_t inbound_size = 8;
	const uint32_t outbound_size = 8;

void init(void) {
	window = window_create();
  
 window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  
  window_set_click_config_provider(window, click_config_provider);
  
  
  #ifdef PBL_COLOR
    // Specify custom Window color using byte
    window_set_background_color(window, (GColor8){ .argb = 0b11010101 });
  #else
    window_set_background_color(window, GColorBlack);
  #endif
 
	window_stack_push(window, true);
 
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
  
	app_message_open(app_message_inbox_size_maximum(),
                              app_message_outbox_size_maximum() );
	
	send_message("hola");
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}