// YAML.node, © 2012 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

#include <node.h>
#include <v8.h>
#include <yaml.h>
#include "nan.h"

using namespace v8;
using namespace node;

namespace yaml {


// Event types.
static Persistent<String> stream_start_symbol;
static Persistent<String> stream_end_symbol;
static Persistent<String> document_start_symbol;
static Persistent<String> document_end_symbol;
static Persistent<String> alias_symbol;
static Persistent<String> scalar_symbol;
static Persistent<String> sequence_start_symbol;
static Persistent<String> sequence_end_symbol;
static Persistent<String> mapping_start_symbol;
static Persistent<String> mapping_end_symbol;

// Mark attributes.
static Persistent<String> index_symbol;
static Persistent<String> line_symbol;
static Persistent<String> column_symbol;

// Event attributes.
static Persistent<String> type_symbol;
static Persistent<String> start_symbol;
static Persistent<String> end_symbol;
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

// Error attributes.
static Persistent<String> offset_symbol;
//static Persistent<String> value_symbol;
static Persistent<String> context_symbol;
static Persistent<String> problem_symbol;
static Persistent<String> description_symbol;


// Convert from LibYAML's booleans.
static inline Handle<Boolean>
BoolToJs(int value)
{
  return value ? True() : False();
}


// Convert from LibYAML's strings.
static inline Handle<Value>
StringToJs(yaml_char_t *value)
{
  if (value) {
    return String::New((const char *)value);
  } else {
    return Null();
  }
}

static inline Handle<Value>
StringToJs(yaml_char_t *value, size_t length)
{
  if (value) {
    return String::New((const char *)value, (int)length);
  } else {
    return Null();
  }
}


// Create an object from LibYAML's mark.
static inline Local<Object>
MarkToJs(yaml_mark_t &mark)
{
  Local<Object> obj = Object::New();
  obj->Set(NanPersistentToLocal(index_symbol),  Integer::NewFromUnsigned(mark.index));
  obj->Set(NanPersistentToLocal(line_symbol),   Integer::NewFromUnsigned(mark.line));
  obj->Set(NanPersistentToLocal(column_symbol), Integer::NewFromUnsigned(mark.column));
  return obj;
}


// Create an object from a LibYAML event.
static inline Local<Object>
EventToJs(yaml_event_t &event)
{
  Local<Object> obj, tmp;

  // Create the event object.
  obj = Object::New();
  obj->Set(NanPersistentToLocal(start_symbol), MarkToJs(event.start_mark));
  obj->Set(NanPersistentToLocal(end_symbol),   MarkToJs(event.end_mark));

  switch (event.type) {
    case YAML_STREAM_START_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(stream_start_symbol));
      break;

    case YAML_STREAM_END_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(stream_end_symbol));
      break;

    case YAML_DOCUMENT_START_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(document_start_symbol));
      if (event.data.document_start.version_directive) {
        tmp = Object::New();
        tmp->Set(NanPersistentToLocal(major_symbol), Integer::New(event.data.document_start.version_directive->major));
        tmp->Set(NanPersistentToLocal(minor_symbol), Integer::New(event.data.document_start.version_directive->minor));
        obj->Set(NanPersistentToLocal(version_symbol), tmp);
      }
      else
        obj->Set(NanPersistentToLocal(version_symbol), Null());
      obj->Set(NanPersistentToLocal(implicit_symbol), BoolToJs(event.data.document_start.implicit));
      break;

    case YAML_DOCUMENT_END_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(document_end_symbol));
      obj->Set(NanPersistentToLocal(implicit_symbol), BoolToJs(event.data.document_end.implicit));
      break;

    case YAML_ALIAS_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(alias_symbol));
      obj->Set(NanPersistentToLocal(anchor_symbol), StringToJs(event.data.alias.anchor));
      break;

    case YAML_SCALAR_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(scalar_symbol));
      obj->Set(NanPersistentToLocal(anchor_symbol), StringToJs(event.data.scalar.anchor));
      obj->Set(NanPersistentToLocal(tag_symbol),    StringToJs(event.data.scalar.tag));
      obj->Set(NanPersistentToLocal(value_symbol),  StringToJs(event.data.scalar.value, event.data.scalar.length));
      obj->Set(NanPersistentToLocal(plain_implicit_symbol),  BoolToJs(event.data.scalar.plain_implicit));
      obj->Set(NanPersistentToLocal(quoted_implicit_symbol), BoolToJs(event.data.scalar.quoted_implicit));
      switch (event.data.scalar.style) {
        case YAML_PLAIN_SCALAR_STYLE:         obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(plain_symbol));         break;
        case YAML_SINGLE_QUOTED_SCALAR_STYLE: obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(single_quoted_symbol)); break;
        case YAML_DOUBLE_QUOTED_SCALAR_STYLE: obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(double_quoted_symbol)); break;
        case YAML_LITERAL_SCALAR_STYLE:       obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(literal_symbol));       break;
        case YAML_FOLDED_SCALAR_STYLE:        obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(folded_symbol));        break;
        default: break;
      }
      break;

    case YAML_SEQUENCE_START_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(sequence_start_symbol));
      obj->Set(NanPersistentToLocal(anchor_symbol),    StringToJs(event.data.sequence_start.anchor));
      obj->Set(NanPersistentToLocal(tag_symbol),       StringToJs(event.data.sequence_start.tag));
      obj->Set(NanPersistentToLocal(implicit_symbol), BoolToJs(event.data.sequence_start.implicit));
      switch (event.data.sequence_start.style) {
        case YAML_BLOCK_SEQUENCE_STYLE: obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(block_symbol)); break;
        case YAML_FLOW_SEQUENCE_STYLE:  obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(flow_symbol));  break;
        default: break;
      }
      break;

    case YAML_SEQUENCE_END_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(sequence_end_symbol));
      break;

    case YAML_MAPPING_START_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(mapping_start_symbol));
      obj->Set(NanPersistentToLocal(anchor_symbol),    StringToJs(event.data.mapping_start.anchor));
      obj->Set(NanPersistentToLocal(tag_symbol),       StringToJs(event.data.mapping_start.tag));
      obj->Set(NanPersistentToLocal(implicit_symbol), BoolToJs(event.data.mapping_start.implicit));
      switch (event.data.mapping_start.style) {
        case YAML_BLOCK_MAPPING_STYLE: obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(block_symbol)); break;
        case YAML_FLOW_MAPPING_STYLE:  obj->Set(NanPersistentToLocal(style_symbol), NanPersistentToLocal(flow_symbol));  break;
        default: break;
      }
      break;

    case YAML_MAPPING_END_EVENT:
      obj->Set(NanPersistentToLocal(type_symbol), NanPersistentToLocal(mapping_end_symbol));
      break;

    default:
      obj->Set(NanPersistentToLocal(type_symbol), Null());
      break;
  }

  return obj;
}


// Create a LibYAML event from an input object.
static inline yaml_event_t *
JsToEvent(Local<Object> &obj)
{
  yaml_event_t *event = new yaml_event_t;
  event->type = YAML_NO_EVENT;

  Local<Value> type = obj->Get(NanPersistentToLocal(type_symbol));

  if (type->StrictEquals(NanPersistentToLocal(stream_start_symbol))) {
    // FIXME: Detect endianness?
    yaml_stream_start_event_initialize(event, YAML_ANY_ENCODING);
  }

  else if (type->StrictEquals(NanPersistentToLocal(stream_end_symbol))) {
    yaml_stream_end_event_initialize(event);
  }

  else if (type->StrictEquals(NanPersistentToLocal(document_start_symbol))) {
    yaml_document_start_event_initialize(event, NULL, NULL, NULL, 0);
  }

  else if (type->StrictEquals(NanPersistentToLocal(document_end_symbol))) {
    yaml_document_end_event_initialize(event, 0);
  }

  else if (type->StrictEquals(NanPersistentToLocal(alias_symbol))) {
    Local<Value> tmp = obj->Get(NanPersistentToLocal(anchor_symbol));
    if (!tmp->IsString()) goto end;
    String::AsciiValue anchor(tmp->ToString());

    yaml_alias_event_initialize(event, (yaml_char_t *)*anchor);
  }

  else if (type->StrictEquals(NanPersistentToLocal(scalar_symbol))) {
    Local<Value> tmp = obj->Get(NanPersistentToLocal(value_symbol));
    if (!tmp->IsString()) goto end;
    String::AsciiValue value(tmp->ToString());

    yaml_scalar_event_initialize(event, NULL, NULL,
        (yaml_char_t *)*value, value.length(),
        1, 1, YAML_ANY_SCALAR_STYLE);
  }

  else if (type->StrictEquals(NanPersistentToLocal(sequence_start_symbol))) {
    yaml_sequence_start_event_initialize(event, NULL, NULL, 0, YAML_ANY_SEQUENCE_STYLE);
  }

  else if (type->StrictEquals(NanPersistentToLocal(sequence_end_symbol))) {
    yaml_sequence_end_event_initialize(event);
  }

  else if (type->StrictEquals(NanPersistentToLocal(mapping_start_symbol))) {
    yaml_mapping_start_event_initialize(event, NULL, NULL, 0, YAML_ANY_MAPPING_STYLE);
  }

  else if (type->StrictEquals(NanPersistentToLocal(mapping_end_symbol))) {
    yaml_mapping_end_event_initialize(event);
  }

end:
  return event;
}


// Create an Error from the LibYAML parser state.
static inline Local<Value>
ParserErrorToJs(yaml_parser_t &parser)
{
  Local<String> problem = String::New(
      parser.problem ? parser.problem : "Unknown error");

  Local<Object> error, mark;
  switch (parser.error) {
    case YAML_READER_ERROR:
      problem = String::Concat(problem, String::New(", at byte offset "));
      problem = String::Concat(problem,
          Integer::NewFromUnsigned(parser.problem_offset)->ToString());
      error = Local<Object>::Cast(Exception::Error(problem));

      mark = Object::New();
      mark->Set(NanPersistentToLocal(offset_symbol), Integer::NewFromUnsigned(parser.problem_offset));
      mark->Set(NanPersistentToLocal(value_symbol), Integer::NewFromUnsigned(parser.problem_value));
      if (parser.problem != NULL)
        mark->Set(NanPersistentToLocal(description_symbol), String::New(parser.problem));
      error->Set(NanPersistentToLocal(problem_symbol), mark);

      break;

    case YAML_SCANNER_ERROR:
    case YAML_PARSER_ERROR:
      if (parser.context != NULL) {
        problem = String::Concat(problem, String::New(", "));
        problem = String::Concat(problem, String::New(parser.context));
      }
      if (parser.problem != NULL) {
        problem = String::Concat(problem, String::New(", on line "));
        problem = String::Concat(problem,
            Integer::NewFromUnsigned(parser.problem_mark.line)->ToString());
      }
      error = Local<Object>::Cast(Exception::Error(problem));

      if (parser.context != NULL) {
        mark = MarkToJs(parser.context_mark);
        mark->Set(NanPersistentToLocal(description_symbol), String::New(parser.context));
        error->Set(NanPersistentToLocal(context_symbol), mark);
      }

      if (parser.problem != NULL) {
        mark = MarkToJs(parser.problem_mark);
        mark->Set(NanPersistentToLocal(description_symbol), String::New(parser.problem));
        error->Set(NanPersistentToLocal(problem_symbol), mark);
      }

      break;

    default:
      break;
  }

  return error;
}


// Create an Error from the LibYAML emitter state.
static inline Local<Value>
EmitterErrorToJs(yaml_emitter_t &emitter)
{
  if (emitter.error == YAML_EMITTER_ERROR)
    return Exception::Error(String::New(emitter.problem));
  else
    return Exception::Error(String::New("Unknown error"));
}


// Binding to LibYAML's stream parser. The function signature is:
//
//     parse(input, handler);
//
// Where `input` is a string, and `handler` is a function receiving events.
static NAN_METHOD(Parse)
{
  NanScope();

  // Check arguments.
  if (args.Length() != 2)
    return NanThrowError("Two arguments were expected.");
  if (!args[0]->IsString())
    return NanThrowTypeError("Input must be a string.");
  if (!args[1]->IsFunction())
    return NanThrowTypeError("Handler must be a function.");

  // Dereference arguments.
  String::Value value(args[0]);
  const uint16_t *input = *value;
  size_t size = value.length();
  Local<Function> handler = Local<Function>::Cast(args[1]);

  // Strip the BOM.
  if (size != 0 && input[0] == 0xFEFF) {
    input++;
    size--;
  }

  // LibYAML expects a UTF-16 character array.
  const unsigned char *string = (const unsigned char *)input;
  size *= sizeof(uint16_t);

  // Initialize parser.
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
    return NanThrowError("Could not initiaize libYAML");
  // FIXME: Detect endianness?
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  yaml_parser_set_input_string(&parser, string, size);

  // Event loop.
  yaml_event_t event;
  while (true) {
    // Get the next event, or throw an exception.
    if (yaml_parser_parse(&parser, &event) == 0)
      return NanThrowError(ParserErrorToJs(parser));

    // Call the handler method.
    Local<Value> params[1] = { EventToJs(event) };
    handler->Call(Context::GetCurrent()->Global(), 1, params);

    // Clean up the event.
    if (event.type == YAML_STREAM_END_EVENT) {
      yaml_event_delete(&event);
      break;
    }
    else {
      yaml_event_delete(&event);
    }
  }

  // Clean up the parser.
  yaml_parser_delete(&parser);

  NanReturnUndefined();
}


// Binding to LibYAML's stream emitter. The usage is more or less the opposite of `parse`:
//
//     var emitter = new Emitter(function(data) { /* ... */ };
//     emitter.event({ type: 'streamStart' });
//
// The constructor takes a function called with output data as LibYAML provides it.
class Emitter : ObjectWrap
{
public:
  static void
  Initialize(Handle<Object> target)
  {
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "event", Event);

    target->Set(String::NewSymbol("Emitter"), t->GetFunction());
  }

  virtual
  ~Emitter()
  {
    writeCallback_.Dispose();
    yaml_emitter_delete(&emitter_);
  }

private:
  Emitter() {}

  static NAN_METHOD(New)
  {
    if (args.Length() != 1)
      return NanThrowTypeError("Expected one argument");
    if (!args[0]->IsFunction())
      return NanThrowTypeError("Expected a function");

    Emitter *e = new Emitter();

    if (!yaml_emitter_initialize(&e->emitter_))
      return NanThrowError("Could not initiaize libYAML");
    // FIXME: Detect endianness?
    yaml_emitter_set_encoding(&e->emitter_, YAML_UTF16LE_ENCODING);
    yaml_emitter_set_output(&e->emitter_, WriteHandler, e);

    NanAssignPersistent(Function, e->writeCallback_, args[0].As<Function>());
    e->Wrap(args.This());
    NanReturnValue(NanObjectWrapHandle(e));
  }

  static NAN_METHOD(Event)
  {
    if (args.Length() != 1)
      return NanThrowTypeError("Expected one argument");
    if (!args[0]->IsObject())
      return NanThrowTypeError("Expected an object");
    Local<Object> obj = Local<Object>::Cast(args[0]);

    Emitter *e = ObjectWrap::Unwrap<Emitter>(args.This());

    yaml_event_t *ev = JsToEvent(obj);
    if (yaml_emitter_emit(&e->emitter_, ev) == 0)
      return NanThrowError(EmitterErrorToJs(e->emitter_));

    NanReturnUndefined();
  }

  static int
  WriteHandler(void *data, unsigned char *buffer, size_t size)
  {
    Emitter *e = (Emitter *)data;
    NanScope();

    // V8 expects UTF-16 as uint16_t.
    const uint16_t *string = (const uint16_t *)buffer;
    size /= sizeof(uint16_t);

    // Strip the BOM.
    if (string[0] == 0xFEFF) {
      string++;
      size--;
    }

    // Call the write callback.
    TryCatch try_catch;
    Local<Value> params[1] = { String::New(string, size) };
    NanPersistentToLocal(e->writeCallback_)->Call(Context::GetCurrent()->Global(), 1, params);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
      return 0;
    }

    return 1;
  }

private:
  yaml_emitter_t emitter_;
  Persistent<Function> writeCallback_;
};


// Defines all symbols and module attributes.
static void
Initialize(Handle<Object> target)
{
  NanScope();

  NanAssignPersistent(String, stream_start_symbol, String::NewSymbol("streamStart"));
  NanAssignPersistent(String, stream_end_symbol, String::NewSymbol("streamEnd"));
  NanAssignPersistent(String, document_start_symbol, String::NewSymbol("documentStart"));
  NanAssignPersistent(String, document_end_symbol, String::NewSymbol("documentEnd"));
  NanAssignPersistent(String, alias_symbol, String::NewSymbol("alias"));
  NanAssignPersistent(String, scalar_symbol, String::NewSymbol("scalar"));
  NanAssignPersistent(String, sequence_start_symbol, String::NewSymbol("sequenceStart"));
  NanAssignPersistent(String, sequence_end_symbol, String::NewSymbol("sequenceEnd"));
  NanAssignPersistent(String, mapping_start_symbol, String::NewSymbol("mappingStart"));
  NanAssignPersistent(String, mapping_end_symbol, String::NewSymbol("mappingEnd"));

  NanAssignPersistent(String, index_symbol, String::NewSymbol("index"));
  NanAssignPersistent(String, line_symbol, String::NewSymbol("line"));
  NanAssignPersistent(String, column_symbol, String::NewSymbol("column"));

  NanAssignPersistent(String, start_symbol, String::NewSymbol("start"));
  NanAssignPersistent(String, end_symbol, String::NewSymbol("end"));
  NanAssignPersistent(String, type_symbol, String::NewSymbol("type"));
  NanAssignPersistent(String, major_symbol, String::NewSymbol("major"));
  NanAssignPersistent(String, minor_symbol, String::NewSymbol("minor"));
  NanAssignPersistent(String, version_symbol, String::NewSymbol("version"));
  NanAssignPersistent(String, implicit_symbol, String::NewSymbol("implicit"));
  NanAssignPersistent(String, anchor_symbol, String::NewSymbol("anchor"));
  NanAssignPersistent(String, tag_symbol, String::NewSymbol("tag"));
  NanAssignPersistent(String, value_symbol, String::NewSymbol("value"));
  NanAssignPersistent(String, plain_implicit_symbol, String::NewSymbol("plain_implicit"));
  NanAssignPersistent(String, quoted_implicit_symbol, String::NewSymbol("quoted_implicit"));
  NanAssignPersistent(String, style_symbol, String::NewSymbol("style"));

  NanAssignPersistent(String, plain_symbol, String::NewSymbol("plain"));
  NanAssignPersistent(String, single_quoted_symbol, String::NewSymbol("single-quoted"));
  NanAssignPersistent(String, double_quoted_symbol, String::NewSymbol("double-quoted"));
  NanAssignPersistent(String, literal_symbol, String::NewSymbol("literal"));
  NanAssignPersistent(String, folded_symbol, String::NewSymbol("folded"));

  NanAssignPersistent(String, block_symbol, String::NewSymbol("block"));
  NanAssignPersistent(String, flow_symbol, String::NewSymbol("flow"));

  NanAssignPersistent(String, offset_symbol, String::NewSymbol("offset"));
  // NanAssignPersistent(String, value_symbol, String::NewSymbol("value"));
  NanAssignPersistent(String, context_symbol, String::NewSymbol("context"));
  NanAssignPersistent(String, problem_symbol, String::NewSymbol("problem"));
  NanAssignPersistent(String, description_symbol, String::NewSymbol("description"));

  Local<FunctionTemplate> parse_template = FunctionTemplate::New(Parse);
  target->Set(String::NewSymbol("parse"), parse_template->GetFunction());

  Emitter::Initialize(target);
}


} // namespace yaml


// Entry-point.
extern "C" void
init(Handle<Object> target)
{
  yaml::Initialize(target);
}

NODE_MODULE(binding, init)
