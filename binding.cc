// YAML.node, © 2011 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

#include <v8.h>
#include <node.h>
#include <yaml.h>
#include <stdexcept>

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


// Convert from LibYAML's booleans.
static inline Handle<Boolean>
FromBoolean(int value)
{
  return value ? True() : False();
}


// Convert from LibYAML's strings.
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


// Create an object from LibYAML's mark.
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

  // Check arguments.
  if (args.Length() != 2)
    return ThrowException(Exception::Error(String::New("Two arguments were expected.")));
  if (!args[0]->IsString()) 
    return ThrowException(Exception::TypeError(String::New("Input must be a string.")));
  if (!args[1]->IsObject()) 
    return ThrowException(Exception::TypeError(String::New("Handler must be an object.")));

  // Dereference arguments.
  String::Value value(args[0]);
  const uint16_t *input = *value;
  size_t size = value.length();
  Local<Object> handler = args[1]->ToObject();

  // Strip the BOM.
  if (input[0] == 0xFEFF) {
    input++;
    size--;
  }

  // LibYAML expects a UTF-16 character array.
  const unsigned char *string = (const unsigned char *)input;
  size *= sizeof(uint16_t);

  // Initialize parser.
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
    return ThrowException(Exception::Error(String::New("YAML parser initialization failed.")));
  // FIXME: Detect endianness?
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  yaml_parser_set_input_string(&parser, string, size);

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


class Emitter : ObjectWrap
{
public:
  static void
  Initialize(Handle<Object> target)
  {
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->InstanceTemplate()->SetAccessor(String::NewSymbol("chunks"), GetChunks, NULL);

    NODE_SET_PROTOTYPE_METHOD(t, "stream", Stream);
    NODE_SET_PROTOTYPE_METHOD(t, "document", Document);
    NODE_SET_PROTOTYPE_METHOD(t, "sequence", Sequence);
    NODE_SET_PROTOTYPE_METHOD(t, "mapping", Mapping);
    NODE_SET_PROTOTYPE_METHOD(t, "alias", Alias);
    NODE_SET_PROTOTYPE_METHOD(t, "scalar", Scalar);

    target->Set(String::NewSymbol("Emitter"), t->GetFunction());
  }

  virtual
  ~Emitter()
  {
    chunks_.Dispose();
    yaml_emitter_delete(&emitter_);
  }

private:
  Emitter()
  {
    HandleScope scope;

    if (!yaml_emitter_initialize(&emitter_))
      throw new std::runtime_error("YAML emitter initialization failed.");
    // FIXME: Detect endianness?
    yaml_emitter_set_encoding(&emitter_, YAML_UTF16LE_ENCODING);
    yaml_emitter_set_output(&emitter_, WriteHandler, this);

    chunks_ = Persistent<Array>::New(Array::New());
    chunks_pos_ = 0;
  }

  static Handle<Value>
  New(const Arguments &args)
  {
    Emitter *e;
    try {
      e = new Emitter();
    }
    catch (const std::exception &exc) {
      return ThrowException(Exception::Error(String::New(exc.what())));
    }
    e->Wrap(args.This());
    return e->handle_;
  }

  static inline Emitter *
  GetEmitter(const Arguments &args)
  {
    return ObjectWrap::Unwrap<Emitter>(args.This());
  }

  static Handle<Value>
  Stream(const Arguments &args)
  {
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsFunction())
        return ThrowException(Exception::TypeError(
            String::New("Expected a function")));
    Local<Function> block = Local<Function>::Cast(args[0]);

    Emitter *e = GetEmitter(args);
    yaml_event_t *ev;

    ev = new yaml_event_t;
    yaml_stream_start_event_initialize(ev, YAML_ANY_ENCODING);
    yaml_emitter_emit(&e->emitter_, ev);

    block->Call(Context::GetCurrent()->Global(), 0, NULL);

    ev = new yaml_event_t;
    yaml_stream_end_event_initialize(ev);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static Handle<Value>
  Document(const Arguments &args)
  {
    // FIXME: Event options.
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsFunction())
        return ThrowException(Exception::TypeError(
            String::New("Expected a function")));
    Local<Function> block = Local<Function>::Cast(args[0]);

    Emitter *e = GetEmitter(args);
    yaml_event_t *ev;

    ev = new yaml_event_t;
    yaml_document_start_event_initialize(ev, NULL, NULL, NULL, 0);
    yaml_emitter_emit(&e->emitter_, ev);

    block->Call(Context::GetCurrent()->Global(), 0, NULL);

    ev = new yaml_event_t;
    yaml_document_end_event_initialize(ev, 0);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static Handle<Value>
  Sequence(const Arguments &args)
  {
    // FIXME: Event options.
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsFunction())
        return ThrowException(Exception::TypeError(
            String::New("Expected a function")));
    Local<Function> block = Local<Function>::Cast(args[0]);

    Emitter *e = GetEmitter(args);
    yaml_event_t *ev;

    ev = new yaml_event_t;
    yaml_sequence_start_event_initialize(ev, NULL, NULL, 0, YAML_ANY_SEQUENCE_STYLE);
    yaml_emitter_emit(&e->emitter_, ev);

    block->Call(Context::GetCurrent()->Global(), 0, NULL);

    ev = new yaml_event_t;
    yaml_sequence_end_event_initialize(ev);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static Handle<Value>
  Mapping(const Arguments &args)
  {
    // FIXME: Event options.
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsFunction())
        return ThrowException(Exception::TypeError(
            String::New("Expected a function")));
    Local<Function> block = Local<Function>::Cast(args[0]);

    Emitter *e = GetEmitter(args);
    yaml_event_t *ev;

    ev = new yaml_event_t;
    yaml_mapping_start_event_initialize(ev, NULL, NULL, 0, YAML_ANY_MAPPING_STYLE);
    yaml_emitter_emit(&e->emitter_, ev);

    block->Call(Context::GetCurrent()->Global(), 0, NULL);

    ev = new yaml_event_t;
    yaml_mapping_end_event_initialize(ev);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static Handle<Value>
  Alias(const Arguments &args)
  {
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsString())
        return ThrowException(Exception::TypeError(
            String::New("Expected a string")));
    String::AsciiValue anchor(args[0]->ToString());

    Emitter *e = GetEmitter(args);

    yaml_event_t *ev = new yaml_event_t;
    yaml_alias_event_initialize(ev, (yaml_char_t *)*anchor);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static Handle<Value>
  Scalar(const Arguments &args)
  {
    // FIXME: Event options.
    if (args.Length() != 1)
        return ThrowException(Exception::TypeError(
            String::New("Expected one argument")));
    if (!args[0]->IsString())
        return ThrowException(Exception::TypeError(
            String::New("Expected a string")));
    String::Utf8Value value(args[0]->ToString());

    Emitter *e = GetEmitter(args);

    yaml_event_t *ev = new yaml_event_t;
    yaml_scalar_event_initialize(ev, NULL, NULL,
        (yaml_char_t *)*value, value.length(),
        1, 1, YAML_ANY_SCALAR_STYLE);
    yaml_emitter_emit(&e->emitter_, ev);

    return Undefined();
  }

  static int
  WriteHandler(void *data, unsigned char *buffer, size_t size)
  {
    Emitter *e = (Emitter *)data;
    HandleScope scope;

    // V8 expects UTF-16 as uint16_t.
    const uint16_t *string = (const uint16_t *)buffer;
    size /= sizeof(uint16_t);

    // Strip the BOM.
    if (string[0] == 0xFEFF) {
      string++;
      size--;
    }

    // Append to the array of chunks.
    TryCatch try_catch;
    e->chunks_->Set(e->chunks_pos_++, String::New(string, size));
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
      return 0;
    }

    return 1;
  }

  static Handle<Value>
  GetChunks(Local<String> property, const AccessorInfo &info)
  {
    Emitter *e = ObjectWrap::Unwrap<Emitter>(info.Holder());
    return Local<Array>::New(e->chunks_);
  }

private:
  yaml_emitter_t emitter_;
  Persistent<Array> chunks_;
  size_t chunks_pos_;
};


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

  Emitter::Initialize(target);
}


} // namespace yaml


// Entry-point.
extern "C" void
init(Handle<Object> target)
{
  yaml::Initialize(target);
}
