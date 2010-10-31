// YAML.node, © 2010 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

#include <v8.h>
#include <node.h>
#include <yaml.h>

using namespace v8;
using namespace node;

namespace yaml {


// Handler method names.
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

// Event mark attributes.
static Persistent<String> start_symbol;
static Persistent<String> end_symbol;

// Mark attributes.
static Persistent<String> index_symbol;
static Persistent<String> line_symbol;
static Persistent<String> column_symbol;

// Event attributes.
static Persistent<String> major_symbol;
static Persistent<String> minor_symbol;
static Persistent<String> version_symbol;
static Persistent<String> implicit_symbol;
static Persistent<String> anchor_symbol;
static Persistent<String> tag_symbol;
static Persistent<String> value_symbol;
static Persistent<String> plain_implicit_symbol;
static Persistent<String> quoted_implicit_symbol;
static Persistent<String> style_symbol;

// Scalar styles.
static Persistent<String> plain_symbol;
static Persistent<String> single_quoted_symbol;
static Persistent<String> double_quoted_symbol;
static Persistent<String> literal_symbol;
static Persistent<String> folded_symbol;

// Sequence and mapping styles.
static Persistent<String> block_symbol;
static Persistent<String> flow_symbol;


// Convert from libyaml's booleans.
static inline Handle<Boolean>
FromBoolean(int value)
{
  return value ? True() : False();
}


// Convert from libyaml's strings.
static inline Handle<Value>
FromString(yaml_char_t *value)
{
  return value ? String::New((const char *)value) : Null();
}

static inline Handle<Value>
FromString(yaml_char_t *value, size_t length)
{
  return value ? String::New((const char *)value, (int)length) : Null();
}


// Create an object from libyaml's mark.
static inline Local<Object>
FromMark(yaml_mark_t &mark)
{
  Local<Object> obj = Object::New();
  obj->Set(index_symbol,  Integer::NewFromUnsigned(mark.index));
  obj->Set(line_symbol,   Integer::NewFromUnsigned(mark.line));
  obj->Set(column_symbol, Integer::NewFromUnsigned(mark.column));
  return obj;
}


// The workhorse.
static Handle<Value>
Parse(const Arguments &args)
{
  HandleScope scope;

  // Check arguments
  if (args.Length() != 2)
    return ThrowException(Exception::Error(String::New("Two arguments were expected.")));
  if (!args[0]->IsString()) 
    return ThrowException(Exception::TypeError(String::New("Input must be a string.")));
  if (!args[1]->IsObject()) 
    return ThrowException(Exception::TypeError(String::New("Handler must be an object.")));

  // Dereference arguments.
  String::Value input(args[0]);
  Local<Object> handler = args[1]->ToObject();

  // Initialize parser.
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
    return ThrowException(Exception::Error(String::New("YAML parser initialization failed.")));
  // FIXME: Detect endianness?
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  yaml_parser_set_input_string(&parser,
      (const unsigned char *)*input, input.length() * sizeof(uint16_t));

  // Event loop.
  yaml_event_t event;
  Local<String> method_name;
  Local<Value> method;
  Local<Object> obj, tmp;
  Local<Value> params[1];
  while (yaml_parser_parse(&parser, &event)) {
    // Find the right handler method.
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

    // Create the event object.
    obj = Object::New();
    obj->Set(start_symbol, FromMark(event.start_mark));
    obj->Set(end_symbol,   FromMark(event.end_mark));

    switch (event.type) {
      case YAML_DOCUMENT_START_EVENT:
        if (event.data.document_start.version_directive) {
          tmp = Object::New();
          tmp->Set(major_symbol, Integer::New(event.data.document_start.version_directive->major));
          tmp->Set(minor_symbol, Integer::New(event.data.document_start.version_directive->minor));
          obj->Set(version_symbol, tmp);
        }
        else
          obj->Set(version_symbol, Null());
        obj->Set(implicit_symbol, FromBoolean(event.data.document_start.implicit));
        break;

      case YAML_DOCUMENT_END_EVENT:
        obj->Set(implicit_symbol, FromBoolean(event.data.document_end.implicit));
        break;

      case YAML_ALIAS_EVENT:
        obj->Set(anchor_symbol, FromString(event.data.alias.anchor));
        break;

      case YAML_SCALAR_EVENT:
        obj->Set(anchor_symbol, FromString(event.data.scalar.anchor));
        obj->Set(tag_symbol,    FromString(event.data.scalar.tag));
        obj->Set(value_symbol,  FromString(event.data.scalar.value, event.data.scalar.length));
        obj->Set(plain_implicit_symbol,  FromBoolean(event.data.scalar.plain_implicit));
        obj->Set(quoted_implicit_symbol, FromBoolean(event.data.scalar.quoted_implicit));
        switch (event.data.scalar.style) {
          case YAML_PLAIN_SCALAR_STYLE:         obj->Set(style_symbol, plain_symbol);         break;
          case YAML_SINGLE_QUOTED_SCALAR_STYLE: obj->Set(style_symbol, single_quoted_symbol); break;
          case YAML_DOUBLE_QUOTED_SCALAR_STYLE: obj->Set(style_symbol, double_quoted_symbol); break;
          case YAML_LITERAL_SCALAR_STYLE:       obj->Set(style_symbol, literal_symbol);       break;
          case YAML_FOLDED_SCALAR_STYLE:        obj->Set(style_symbol, folded_symbol);        break;
          default: break;
        }
        break;

      case YAML_SEQUENCE_START_EVENT:
        obj->Set(anchor_symbol,    FromString(event.data.sequence_start.anchor));
        obj->Set(tag_symbol,       FromString(event.data.sequence_start.tag));
        obj->Set(implicit_symbol, FromBoolean(event.data.sequence_start.implicit));
        switch (event.data.sequence_start.style) {
          case YAML_BLOCK_SEQUENCE_STYLE: obj->Set(style_symbol, block_symbol); break;
          case YAML_FLOW_SEQUENCE_STYLE:  obj->Set(style_symbol, flow_symbol);  break;
          default: break;
        }
        break;

      case YAML_MAPPING_START_EVENT:
        obj->Set(anchor_symbol,    FromString(event.data.mapping_start.anchor));
        obj->Set(tag_symbol,       FromString(event.data.mapping_start.tag));
        obj->Set(implicit_symbol, FromBoolean(event.data.mapping_start.implicit));
        switch (event.data.mapping_start.style) {
          case YAML_BLOCK_MAPPING_STYLE: obj->Set(style_symbol, block_symbol); break;
          case YAML_FLOW_MAPPING_STYLE:  obj->Set(style_symbol, flow_symbol);  break;
          default: break;
        }
        break;

      default:
        break;
    }

    // Call the handler method.
    params[0] = obj;
    Handle<Function>::Cast(method)->Call(handler, 1, params);

  loop_end:
    // Clean up the event.
    if (event.type == YAML_STREAM_END_EVENT) {
      yaml_event_delete(&event);
      break;
    }
    else
      yaml_event_delete(&event);
  }

  // Clean up the parser.
  yaml_parser_delete(&parser);

  return Undefined();
}


// Defines all symbols and module attributes.
static void
Initialize(Handle<Object> target)
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
  tag_symbol      = NODE_PSYMBOL("tag");
  value_symbol    = NODE_PSYMBOL("value");
  plain_implicit_symbol  = NODE_PSYMBOL("plain_implicit");
  quoted_implicit_symbol = NODE_PSYMBOL("quoted_implicit");
  style_symbol    = NODE_PSYMBOL("style");

  plain_symbol   = NODE_PSYMBOL("plain");
  single_quoted_symbol = NODE_PSYMBOL("single-quoted");
  double_quoted_symbol = NODE_PSYMBOL("double-quoted");
  literal_symbol = NODE_PSYMBOL("literal");
  folded_symbol  = NODE_PSYMBOL("folded");

  block_symbol = NODE_PSYMBOL("block");
  flow_symbol  = NODE_PSYMBOL("flow");

  Local<FunctionTemplate> parse_template = FunctionTemplate::New(Parse);
  target->Set(String::NewSymbol("parse"), parse_template->GetFunction());
}


} // namespace yaml


// Entry-point.
extern "C" void
init(Handle<Object> target)
{
  yaml::Initialize(target);
}
