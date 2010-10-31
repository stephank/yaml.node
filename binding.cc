/*
 * Copyright (c) 2010 Stéphan Kochen
 * MIT-licensed. (See the included LICENSE file.)
 */

#include <v8.h>
#include <node.h>
#include <yaml.h>

using namespace v8;
using namespace node;


static Persistent<String> on_stream_start_symbol;
static Persistent<String> on_stream_end_symbol;
static Persistent<String> on_document_start_symbol;
static Persistent<String> on_document_end_symbol;
static Persistent<String> on_alias_symbol;
static Persistent<String> on_scalar_symbol;
static Persistent<String> on_sequence_start_symbol;
static Persistent<String> on_sequence_end_symbol;
static Persistent<String> on_mapping_start_symbol;
static Persistent<String> on_mapping_end_symbol;

static Persistent<String> start_symbol;
static Persistent<String> end_symbol;

static Persistent<String> index_symbol;
static Persistent<String> line_symbol;
static Persistent<String> column_symbol;

static Persistent<String> major_symbol;
static Persistent<String> minor_symbol;
static Persistent<String> version_symbol;
static Persistent<String> implicit_symbol;
static Persistent<String> anchor_symbol;


/* Create an object like libyaml's mark. */
static Local<Object>
yaml_node_object_from_mark(yaml_mark_t &mark)
{
  HandleScope scope;
  Local<Object> obj = Object::New();
  obj->Set(index_symbol,  Integer::NewFromUnsigned(mark.index));
  obj->Set(line_symbol,   Integer::NewFromUnsigned(mark.line));
  obj->Set(column_symbol, Integer::NewFromUnsigned(mark.column));
  return scope.Close(obj);
}


/* The workhorse. */
static Handle<Value>
yaml_node_parse(const Arguments &args)
{
  HandleScope scope;

  /* Check arguments */
  if (args.Length() != 2)
    return ThrowException(Exception::Error(String::New("Two arguments were expected.")));
  if (!args[0]->IsString()) 
    return ThrowException(Exception::TypeError(String::New("Input must be a string.")));
  if (!args[1]->IsObject()) 
    return ThrowException(Exception::TypeError(String::New("Handler must be an object.")));

  /* Dereference arguments. */
  String::Value input(args[0]);
  Local<Object> handler = args[1]->ToObject();

  /* Initialize parser. */
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
    return ThrowException(Exception::Error(String::New("YAML parser initialization failed.")));
  /* FIXME: Detect endianness? */
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  yaml_parser_set_input_string(&parser,
      (const unsigned char *)*input, input.length() * sizeof(uint16_t));

  /* Event loop. */
  yaml_event_t event;
  Local<String> method_name;
  Local<Value> method;
  Local<Object> event_obj, tmp;
  Local<Value> params[1];
  while (yaml_parser_parse(&parser, &event)) {
    /* Find the right handler method. */
    switch (event.type) {
      case YAML_STREAM_START_EVENT:   method_name = *on_stream_start_symbol;   break;
      case YAML_STREAM_END_EVENT:     method_name = *on_stream_end_symbol;     break;
      case YAML_DOCUMENT_START_EVENT: method_name = *on_document_start_symbol; break;
      case YAML_DOCUMENT_END_EVENT:   method_name = *on_document_end_symbol;   break;
      case YAML_ALIAS_EVENT:          method_name = *on_alias_symbol;          break;
      case YAML_SCALAR_EVENT:         method_name = *on_scalar_symbol;         break;
      case YAML_SEQUENCE_START_EVENT: method_name = *on_sequence_start_symbol; break;
      case YAML_SEQUENCE_END_EVENT:   method_name = *on_sequence_end_symbol;   break;
      case YAML_MAPPING_START_EVENT:  method_name = *on_mapping_start_symbol;  break;
      case YAML_MAPPING_END_EVENT:    method_name = *on_mapping_end_symbol;    break;
      default: goto loop_end;
    }
    method = handler->Get(method_name);
    if (!method->IsFunction())
      goto loop_end;

    /* Create the event object. */
    event_obj = Object::New();
    event_obj->Set(start_symbol, yaml_node_object_from_mark(event.start_mark));
    event_obj->Set(end_symbol,   yaml_node_object_from_mark(event.end_mark));

    switch (event.type) {
      case YAML_DOCUMENT_START_EVENT:
        if (event.data.document_start.version_directive) {
          tmp = Object::New();
          tmp->Set(major_symbol, Integer::New(event.data.document_start.version_directive->major));
          tmp->Set(minor_symbol, Integer::New(event.data.document_start.version_directive->minor));
          event_obj->Set(version_symbol, tmp);
        }
        else
          event_obj->Set(version_symbol, Null());
        event_obj->Set(implicit_symbol, event.data.document_start.implicit ? True() : False());
        break;

      case YAML_DOCUMENT_END_EVENT:
        event_obj->Set(implicit_symbol, event.data.document_end.implicit ? True() : False());
        break;

      case YAML_ALIAS_EVENT:
        event_obj->Set(anchor_symbol,
            event.data.alias.anchor ? String::New((const char *)event.data.alias.anchor) : Null());
        break;

      case YAML_SCALAR_EVENT:
        event_obj->Set(anchor_symbol,
            event.data.scalar.anchor ? String::New((const char *)event.data.scalar.anchor) : Null());
        break;

      case YAML_SEQUENCE_START_EVENT:
        break;

      case YAML_SEQUENCE_END_EVENT:
        break;

      case YAML_MAPPING_START_EVENT:
        break;

      case YAML_MAPPING_END_EVENT:
        break;

      default:
        break;
    }

    /* Call the handler method. */
    params[0] = event_obj;
    Handle<Function>::Cast(method)->Call(handler, 1, params);

  loop_end:
    /* Clean up the event. */
    if (event.type == YAML_STREAM_END_EVENT) {
      yaml_event_delete(&event);
      break;
    }
    else
      yaml_event_delete(&event);
  }

  /* Clean up the parser. */
  yaml_parser_delete(&parser);

  return Undefined();
}


extern "C" void
init(Handle<Object> target)
{
  HandleScope scope;

  on_stream_start_symbol   = NODE_PSYMBOL("onStreamStart");
  on_stream_end_symbol     = NODE_PSYMBOL("onStreamEnd");
  on_document_start_symbol = NODE_PSYMBOL("onDocumentStart");
  on_document_end_symbol   = NODE_PSYMBOL("onDocumentEnd");
  on_alias_symbol          = NODE_PSYMBOL("onAlias");
  on_scalar_symbol         = NODE_PSYMBOL("onScalar");
  on_sequence_start_symbol = NODE_PSYMBOL("onSequenceStart");
  on_sequence_end_symbol   = NODE_PSYMBOL("onSequenceEnd");
  on_mapping_start_symbol  = NODE_PSYMBOL("onMappingStart");
  on_mapping_end_symbol    = NODE_PSYMBOL("onMappingEnd");

  start_symbol = NODE_PSYMBOL("start");
  end_symbol   = NODE_PSYMBOL("end");

  index_symbol  = NODE_PSYMBOL("index");
  line_symbol   = NODE_PSYMBOL("line");
  column_symbol = NODE_PSYMBOL("column");

  major_symbol    = NODE_PSYMBOL("major");
  minor_symbol    = NODE_PSYMBOL("minor");
  version_symbol  = NODE_PSYMBOL("version");
  implicit_symbol = NODE_PSYMBOL("implicit");
  anchor_symbol   = NODE_PSYMBOL("anchor");

  Local<FunctionTemplate> parse_template = FunctionTemplate::New(yaml_node_parse);
  target->Set(String::NewSymbol("parse"), parse_template->GetFunction());
}
