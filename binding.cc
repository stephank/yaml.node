// YAML.node, © 2012 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <util.h>
#include <util-inl.h>
#include <yaml.h>

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
  return value ? True(Isolate::GetCurrent()) : False(Isolate::GetCurrent());
}


// Convert from LibYAML's strings.
static inline Handle<Value>
StringToJs(yaml_char_t *value)
{
  if( value )
    return String::NewFromUtf8(Isolate::GetCurrent(),(const char *)value);
  else
    return Null(Isolate::GetCurrent());
}

static inline Handle<Value>
StringToJs(yaml_char_t *value, size_t length)
{
  if( value )
    return String::NewFromUtf8(Isolate::GetCurrent(),(const char *)value, String::kNormalString, (int)length);
  else
    return Null(Isolate::GetCurrent());
}


// Create an object from LibYAML's mark.
static inline Local<Object>
MarkToJs(yaml_mark_t &mark)
{
  Isolate* iso = Isolate::GetCurrent();
  Handle<Integer> m_index =  Integer::NewFromUnsigned(iso, mark.index);
  Handle<Integer> m_line = Integer::NewFromUnsigned(iso, mark.line); 
  Handle<Integer> m_column = Integer::NewFromUnsigned(iso, mark.column);
  Handle<String> i_sym = PersistentToLocal(iso,index_symbol);
  Handle<String> l_sym = PersistentToLocal(iso,line_symbol);
  Handle<String> c_sym = PersistentToLocal(iso,column_symbol); 
 Local<Object> obj = Object::New(iso);
  obj->Set(i_sym,  m_index );
  obj->Set(l_sym,  m_line );
  obj->Set(c_sym,  m_column );
  return obj;
}


// Create an object from a LibYAML event.
static inline Local<Object>
EventToJs(yaml_event_t &event)
{
  Local<Object> obj, tmp;
  Isolate* iso = Isolate::GetCurrent();
  Handle<String> start_s = PersistentToLocal(iso,start_symbol);
  Handle<String> end_s = PersistentToLocal(iso,end_symbol);
  Handle<Object> evsm = MarkToJs(event.start_mark);
  Handle<Object> evem = MarkToJs(event.end_mark);

  // Create the event object.
  obj = Object::New(iso);
  obj->Set(start_s, evsm);
  obj->Set(end_s,   evem);

  Handle<String> ts = PersistentToLocal(iso,type_symbol);
  Handle<String> sss = PersistentToLocal(iso,stream_start_symbol);
  Handle<String> ses = PersistentToLocal(iso,stream_end_symbol);
  Handle<String> dss = PersistentToLocal(iso,document_start_symbol);
  Handle<String> des = PersistentToLocal(iso,document_end_symbol);
  Handle<String> imps = PersistentToLocal(iso,implicit_symbol);
  //Handle<String> majs = PersistentToLocal(iso,major_symbol);
  //Handle<String> mins = PersistentToLocal(iso,minor_symbol);
  Handle<String> vers = PersistentToLocal(iso,version_symbol);
  Handle<String> as = PersistentToLocal(iso,alias_symbol);
  Handle<String> scals = PersistentToLocal(iso,scalar_symbol);
  Handle<String> anchors = PersistentToLocal(iso,anchor_symbol);
  Handle<String> tags = PersistentToLocal(iso,tag_symbol);
  Handle<String> pis = PersistentToLocal(iso,plain_implicit_symbol);
  Handle<String> qis = PersistentToLocal(iso,quoted_implicit_symbol);
  Handle<String> vals = PersistentToLocal(iso,value_symbol);
  Handle<String> styles = PersistentToLocal(iso,style_symbol);
  Handle<String> plains = PersistentToLocal(iso,plain_symbol);
  Handle<String> sqs = PersistentToLocal(iso,single_quoted_symbol);
  Handle<String> dqs = PersistentToLocal(iso,double_quoted_symbol);
  Handle<String> lits = PersistentToLocal(iso,literal_symbol);
  Handle<String> folds = PersistentToLocal(iso,folded_symbol);
  Handle<String> blocks = PersistentToLocal(iso,block_symbol);
  Handle<String> flows = PersistentToLocal(iso,flow_symbol);
  Handle<String> mss = PersistentToLocal(iso,mapping_start_symbol);
  Handle<String> mes = PersistentToLocal(iso,mapping_end_symbol);
  Handle<String> seq_starts = PersistentToLocal(iso,sequence_start_symbol);
  Handle<String> seq_ends = PersistentToLocal(iso,sequence_end_symbol);
 
  //Handle<Integer> maj = Integer::New(iso,event.data.document_start.version_directive->major);
  //Handle<Integer> min = Integer::New(iso,event.data.document_start.version_directive->minor);
  switch (event.type) {
    case YAML_STREAM_START_EVENT:
      obj->Set(ts, sss);
      break;

    case YAML_STREAM_END_EVENT:
      obj->Set(ts, ses);
      break;

    case YAML_DOCUMENT_START_EVENT:
      obj->Set(ts, dss);
      if (event.data.document_start.version_directive) {
        tmp = Object::New(iso);
	//        tmp->Set(majs, maj);
        //tmp->Set(mins, min);
        obj->Set(vers, tmp);
      }
      else
        obj->Set(vers, Null(iso));
      obj->Set(imps, BoolToJs(event.data.document_start.implicit));
      break;

    case YAML_DOCUMENT_END_EVENT:
      obj->Set(ts, des);
      obj->Set(imps, BoolToJs(event.data.document_end.implicit));
      break;

    case YAML_ALIAS_EVENT:
      obj->Set(ts, as);
      obj->Set(anchors, StringToJs(event.data.alias.anchor));
      break;

    case YAML_SCALAR_EVENT:
      obj->Set(ts, scals);
      obj->Set(anchors, StringToJs(event.data.scalar.anchor));
      obj->Set(tags,    StringToJs(event.data.scalar.tag));
      obj->Set(vals,  StringToJs(event.data.scalar.value, event.data.scalar.length));
      obj->Set(pis,  BoolToJs(event.data.scalar.plain_implicit));
      obj->Set(qis, BoolToJs(event.data.scalar.quoted_implicit));
      switch (event.data.scalar.style) {
        case YAML_PLAIN_SCALAR_STYLE:   obj->Set(styles, plains); break;
      case YAML_SINGLE_QUOTED_SCALAR_STYLE: obj->Set(styles,sqs); break;
      case YAML_DOUBLE_QUOTED_SCALAR_STYLE: obj->Set(styles,dqs); break;
      case YAML_LITERAL_SCALAR_STYLE: obj->Set(styles,lits);      break;
      case YAML_FOLDED_SCALAR_STYLE: obj->Set(styles,folds);      break;
        default: break;
      }
      break;

    case YAML_SEQUENCE_START_EVENT:
      obj->Set(ts,seq_starts);
      obj->Set(anchors,    StringToJs(event.data.sequence_start.anchor));
      obj->Set(tags,       StringToJs(event.data.sequence_start.tag));
      obj->Set(imps, BoolToJs(event.data.sequence_start.implicit));
      switch (event.data.sequence_start.style) {
      case YAML_BLOCK_SEQUENCE_STYLE: obj->Set(styles,blocks); break;
      case YAML_FLOW_SEQUENCE_STYLE:  obj->Set(styles,flows);  break;
        default: break;
      }
      break;

    case YAML_SEQUENCE_END_EVENT:
      obj->Set(ts,seq_ends);
      break;

    case YAML_MAPPING_START_EVENT:
      obj->Set(ts,mss);
      obj->Set(anchors,    StringToJs(event.data.mapping_start.anchor));
      obj->Set(tags,       StringToJs(event.data.mapping_start.tag));
      obj->Set(imps, BoolToJs(event.data.mapping_start.implicit));
      switch (event.data.mapping_start.style) {
        case YAML_BLOCK_MAPPING_STYLE: obj->Set(styles, blocks); break;
        case YAML_FLOW_MAPPING_STYLE:  obj->Set(styles, flows);  break;
        default: break;
      }
      break;

    case YAML_MAPPING_END_EVENT:
      obj->Set(ts,mes);
      break;

    default:
      obj->Set(ts, Null(iso));
      break;
  }

  return obj;
}


// Create a LibYAML event from an input object.
static inline yaml_event_t *
JsToEvent(Local<Object> &obj)
{
  Isolate* iso = Isolate::GetCurrent();
  Handle<String> ts = PersistentToLocal(iso,type_symbol);
  Handle<String> sss = PersistentToLocal(iso,stream_start_symbol);
  Handle<String> ses = PersistentToLocal(iso,stream_end_symbol);
  Handle<String> dss = PersistentToLocal(iso,document_start_symbol);
  Handle<String> des = PersistentToLocal(iso,document_end_symbol);
  Handle<String> as = PersistentToLocal(iso,alias_symbol);
  Handle<String> scals = PersistentToLocal(iso,scalar_symbol);
  Handle<String> anchors = PersistentToLocal(iso,anchor_symbol);
  Handle<String> vals = PersistentToLocal(iso,value_symbol);
  Handle<String> mss = PersistentToLocal(iso,mapping_start_symbol);
  Handle<String> mes = PersistentToLocal(iso,mapping_end_symbol);
  Handle<String> seq_starts = PersistentToLocal(iso,sequence_start_symbol);
  Handle<String> seq_ends = PersistentToLocal(iso,sequence_end_symbol);
  yaml_event_t *event = new yaml_event_t;
  event->type = YAML_NO_EVENT;

  Local<Value> type = obj->Get(ts);

  if (type->StrictEquals(sss)) {
    // FIXME: Detect endianness?
    yaml_stream_start_event_initialize(event, YAML_ANY_ENCODING);
  }

  else if (type->StrictEquals(ses)) {
    yaml_stream_end_event_initialize(event);
  }

  else if (type->StrictEquals(dss)) {
    yaml_document_start_event_initialize(event, NULL, NULL, NULL, 0);
  }

  else if (type->StrictEquals(des)) {
    yaml_document_end_event_initialize(event, 0);
  }

  else if (type->StrictEquals(as)) {
    Local<Value> tmp = obj->Get(anchors);
    if (!tmp->IsString()) goto end;
    String::Utf8Value anchor(tmp->ToString());

    yaml_alias_event_initialize(event, (yaml_char_t *)*anchor);
  }

  else if (type->StrictEquals(scals)) {
    Local<Value> tmp = obj->Get(vals);
    if (!tmp->IsString()) goto end;
    String::Utf8Value value(tmp->ToString());

    yaml_scalar_event_initialize(event, NULL, NULL,
        (yaml_char_t *)*value, value.length(),
        1, 1, YAML_ANY_SCALAR_STYLE);
  }

  else if (type->StrictEquals(seq_starts)) {
    yaml_sequence_start_event_initialize(event, NULL, NULL, 0, YAML_ANY_SEQUENCE_STYLE);
  }

  else if (type->StrictEquals(seq_ends)) {
    yaml_sequence_end_event_initialize(event);
  }

  else if (type->StrictEquals(mss)) {
    yaml_mapping_start_event_initialize(event, NULL, NULL, 0, YAML_ANY_MAPPING_STYLE);
  }

  else if (type->StrictEquals(mes)) {
    yaml_mapping_end_event_initialize(event);
  }

end:
  return event;
}


// Create an Error from the LibYAML parser state.
static inline Local<Value>
ParserErrorToJs(yaml_parser_t &parser)
{
  Isolate* iso = Isolate::GetCurrent();
  Handle<String> offset_sym = PersistentToLocal(iso, offset_symbol);
  Handle<String> value_sym = PersistentToLocal(iso, value_symbol);
  Handle<String> desc_sym = PersistentToLocal(iso, description_symbol);
  Handle<String> prob_sym = PersistentToLocal(iso, problem_symbol);
  Handle<String> cont_sym = PersistentToLocal(iso, context_symbol);
  Local<String> problem = String::NewFromUtf8( iso,
      parser.problem ? parser.problem : "Unknown error");

  Local<Object> error, mark;
  switch (parser.error) {
    case YAML_READER_ERROR:
      problem = String::Concat(problem, String::NewFromUtf8(iso,", at byte offset "));
      problem = String::Concat(problem,
			       Integer::NewFromUnsigned(iso,parser.problem_offset)->ToString());
      error = Local<Object>::Cast(Exception::Error(problem));

      mark = Object::New(iso);
      mark->Set(offset_sym, Integer::NewFromUnsigned(iso,parser.problem_offset));
      mark->Set(value_sym, Integer::NewFromUnsigned(iso,parser.problem_value));
      if (parser.problem != NULL)
        mark->Set(desc_sym, String::NewFromUtf8(iso,parser.problem));
      error->Set(prob_sym, mark);

      break;

    case YAML_SCANNER_ERROR:
    case YAML_PARSER_ERROR:
      if (parser.context != NULL) {
        problem = String::Concat(problem, String::NewFromUtf8(iso,", "));
        problem = String::Concat(problem, String::NewFromUtf8(iso,parser.context));
      }
      if (parser.problem != NULL) {
        problem = String::Concat(problem, String::NewFromUtf8(iso,", on line "));
        problem = String::Concat(problem,
				 Integer::NewFromUnsigned(iso,parser.problem_mark.line)->ToString());
      }
      error = Local<Object>::Cast(Exception::Error(problem));

      if (parser.context != NULL) {
        mark = MarkToJs(parser.context_mark);
        mark->Set(desc_sym, String::NewFromUtf8(iso,parser.context));
        error->Set(cont_sym, mark);
      }

      if (parser.problem != NULL) {
        mark = MarkToJs(parser.problem_mark);
        mark->Set(desc_sym, String::NewFromUtf8(iso,parser.problem));
        error->Set(prob_sym, mark);
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
  Isolate* iso = Isolate::GetCurrent();
  if (emitter.error == YAML_EMITTER_ERROR)
    return Exception::Error(String::NewFromUtf8(iso,emitter.problem));
  else
    return Exception::Error(String::NewFromUtf8(iso,"Unknown error"));
}


// Binding to LibYAML's stream parser. The function signature is:
//
//     parse(input, handler);
//
// Where `input` is a string, and `handler` is a function receiving events.
void
Parse(const FunctionCallbackInfo<Value> &args)
{
  Isolate* iso = Isolate::GetCurrent();
  HandleScope scope(iso);

  // Check arguments.
  if (args.Length() != 2) {
    args.GetReturnValue().Set( iso->ThrowException(Exception::Error(
						String::NewFromUtf8(iso,"Two arguments were expected."))));
    return;
  }
  if (!args[0]->IsString()) {
    args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									String::NewFromUtf8(iso,"Input must be a string."))));
    return;
  }
  if (!args[1]->IsFunction()) {
     args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									 String::NewFromUtf8(iso,"Handler must be a function."))));
     return;
  }

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
  if (!yaml_parser_initialize(&parser)) {
     args.GetReturnValue().Set( iso->ThrowException(Exception::Error(
								     String::NewFromUtf8(iso,"Could not initiaize libYAML"))));
     return;
  }
  // FIXME: Detect endianness?
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  yaml_parser_set_input_string(&parser, string, size);

  // Event loop.
  yaml_event_t event;
  while (true) {
    // Get the next event, or throw an exception.
    if (yaml_parser_parse(&parser, &event) == 0) {
      args.GetReturnValue().Set( iso->ThrowException(ParserErrorToJs(parser)));
      break;
    }
    // Call the handler method.
    Local<Value> params[1] = { EventToJs(event) };
    handler->Call(iso->GetCurrentContext()->Global(), 1, params);

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

  //  return Undefined(iso);
}


// Binding to LibYAML's stream emitter. The usage is more or less the opposite of `parse`:
//
//     var emitter = new Emitter(function(data) { /* ... */ };
//     emitter.event({ type: 'streamStart' });
//
// The constructor takes a function called with output data as LibYAML provides it.
class Emitter : public ObjectWrap
{
public:
  static void
  Initialize(Handle<Object> target)
  {
    Isolate* iso = Isolate::GetCurrent();
    Local<FunctionTemplate> t = FunctionTemplate::New(iso,Emitter::New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "event", Event);

    target->Set(String::NewFromUtf8(iso,"Emitter"), t->GetFunction());
  }

  virtual
  ~Emitter()
  {
    writeCallback_.Reset();
    yaml_emitter_delete(&emitter_);
  }

private:
  Emitter() {}

  //  static Handle<Value>
  static void
  New(const FunctionCallbackInfo<Value> &args)
  {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.Length() != 1) {
      args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									  String::NewFromUtf8(iso,"Expected one argument"))));
      return;
    }

    if (!args[0]->IsFunction()) {
      args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									  String::NewFromUtf8(iso,"Expected a function"))));
      return;
    }

    Emitter *e = new Emitter();

    if (!yaml_emitter_initialize(&e->emitter_)) {
      args.GetReturnValue().Set( iso->ThrowException(Exception::Error(
								      String::NewFromUtf8(iso,"Could not initiaize libYAML"))));
      return;
    }

    // FIXME: Detect endianness?
    yaml_emitter_set_encoding(&e->emitter_, YAML_UTF16LE_ENCODING);
    yaml_emitter_set_output(&e->emitter_, WriteHandler, e);

    Handle<Function> hf = Handle<Function>::Cast(args[0]);
    e->writeCallback_.Reset(iso, hf);

    e->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }

  //  static Handle<Value>
  static void
  Event(const FunctionCallbackInfo<Value> &args)
  {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.Length() != 1) {
      args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									  String::NewFromUtf8(iso,"Expected one argument"))));
      return;
    }

    if (!args[0]->IsObject()) {
      args.GetReturnValue().Set( iso->ThrowException(Exception::TypeError(
									  String::NewFromUtf8(iso,"Expected an object"))));
      return;
    }

    Local<Object> obj = Local<Object>::Cast(args[0]);

    Emitter *e = ObjectWrap::Unwrap<Emitter>(args.This());

    yaml_event_t *ev = JsToEvent(obj);
    if (yaml_emitter_emit(&e->emitter_, ev) == 0) {
      args.GetReturnValue().Set(iso->ThrowException(EmitterErrorToJs(e->emitter_)));
      return;
    } else
      args.GetReturnValue().Set(Undefined(iso));
  }

  static int
  WriteHandler(void *data, unsigned char *buffer, size_t size)
  {
    Isolate* iso = Isolate::GetCurrent();
    Emitter *e = (Emitter *)data;
    HandleScope scope(iso);

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
    Local<Value> params[1] = { String::NewFromTwoByte(iso, string, String::kNormalString, (int)size) };
    Handle<Function> f = PersistentToLocal(iso, e->writeCallback_);
    f->Call(iso->GetCurrentContext()->Global(), 1, params);
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
  Isolate* iso = Isolate::GetCurrent();
  HandleScope scope(iso);
  
  stream_start_symbol   .Reset(iso,String::NewFromUtf8(iso,"streamStart"));
  stream_end_symbol     .Reset(iso,String::NewFromUtf8(iso,"streamEnd"));
  document_start_symbol .Reset(iso,String::NewFromUtf8(iso,"documentStart"));
  document_end_symbol   .Reset(iso,String::NewFromUtf8(iso,"documentEnd"));
  alias_symbol          .Reset(iso,String::NewFromUtf8(iso,"alias"));
  scalar_symbol         .Reset(iso,String::NewFromUtf8(iso,"scalar"));
  sequence_start_symbol .Reset(iso,String::NewFromUtf8(iso,"sequenceStart"));
  sequence_end_symbol   .Reset(iso,String::NewFromUtf8(iso,"sequenceEnd"));
  mapping_start_symbol  .Reset(iso,String::NewFromUtf8(iso,"mappingStart"));
  mapping_end_symbol    .Reset(iso,String::NewFromUtf8(iso,"mappingEnd"));

  index_symbol  .Reset(iso,String::NewFromUtf8(iso,"index"));
  line_symbol   .Reset(iso,String::NewFromUtf8(iso,"line"));
  column_symbol .Reset(iso,String::NewFromUtf8(iso,"column"));

  start_symbol    .Reset(iso,String::NewFromUtf8(iso,"start"));
  end_symbol      .Reset(iso,String::NewFromUtf8(iso,"end"));
  type_symbol     .Reset(iso,String::NewFromUtf8(iso,"type"));
  major_symbol    .Reset(iso,String::NewFromUtf8(iso,"major"));
  minor_symbol    .Reset(iso,String::NewFromUtf8(iso,"minor"));
  version_symbol  .Reset(iso,String::NewFromUtf8(iso,"version"));
  implicit_symbol .Reset(iso,String::NewFromUtf8(iso,"implicit"));
  anchor_symbol   .Reset(iso,String::NewFromUtf8(iso,"anchor"));
  tag_symbol      .Reset(iso,String::NewFromUtf8(iso,"tag"));
  value_symbol    .Reset(iso,String::NewFromUtf8(iso,"value"));
  plain_implicit_symbol  .Reset(iso,String::NewFromUtf8(iso,"plain_implicit"));
  quoted_implicit_symbol .Reset(iso,String::NewFromUtf8(iso,"quoted_implicit"));
  style_symbol    .Reset(iso,String::NewFromUtf8(iso,"style"));

  plain_symbol   .Reset(iso,String::NewFromUtf8(iso,"plain"));
  single_quoted_symbol .Reset(iso,String::NewFromUtf8(iso,"single-quoted"));
  double_quoted_symbol .Reset(iso,String::NewFromUtf8(iso,"double-quoted"));
  literal_symbol .Reset(iso,String::NewFromUtf8(iso,"literal"));
  folded_symbol  .Reset(iso,String::NewFromUtf8(iso,"folded"));

  block_symbol .Reset(iso,String::NewFromUtf8(iso,"block"));
  flow_symbol  .Reset(iso,String::NewFromUtf8(iso,"flow"));

  offset_symbol      .Reset(iso,String::NewFromUtf8(iso,"offset"));
  // value_symbol    .Reset(iso,String::NewFromUtf8(iso,"value"));
  context_symbol     .Reset(iso,String::NewFromUtf8(iso,"context"));
  problem_symbol     .Reset(iso,String::NewFromUtf8(iso,"problem"));
  description_symbol .Reset(iso,String::NewFromUtf8(iso,"description"));


  Local<FunctionTemplate> parse_template = FunctionTemplate::New(iso,Parse);
  target->Set(String::NewFromUtf8(iso,"parse"), parse_template->GetFunction());

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
