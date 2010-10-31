/*
 * Copyright (c) 2010 Stéphan Kochen
 * MIT-licensed. (See the included LICENSE file.)
 */

#include <v8.h>
#include <node.h>
#include <yaml.h>

using namespace v8;
using namespace node;

namespace yaml
{

static Handle<Value> ValueFromScalarNode(yaml_document_t *doc, yaml_node_t *node);
static Handle<Value> ValueFromSequenceNode(yaml_document_t *doc, yaml_node_t *node);
static Handle<Value> ValueFromMappingNode(yaml_document_t *doc, yaml_node_t *node);
static Handle<Value> ValueFromNode(yaml_document_t *doc, yaml_node_t *node);
static Handle<Value> Load(const Arguments &args);


static Handle<Value>
ValueFromScalarNode(yaml_document_t *doc, yaml_node_t *node)
{
  return String::New((const char *)node->data.scalar.value, node->data.scalar.length);
}


static Handle<Value>
ValueFromSequenceNode(yaml_document_t *doc, yaml_node_t *node)
{
  uint32_t length = node->data.sequence.items.top - node->data.sequence.items.start;
  Local<Array> result = Array::New(length);
  for (uint32_t i = 0; i < length; i++)
    result->Set(i, ValueFromNode(doc,
        yaml_document_get_node(doc, node->data.sequence.items.start[i])));
  return result;
}


static Handle<Value>
ValueFromMappingNode(yaml_document_t *doc, yaml_node_t *node)
{
  Local<Object> result = Object::New();
  yaml_node_pair_t *pair = node->data.mapping.pairs.start;
  while (pair < node->data.mapping.pairs.top) {
    result->Set(ValueFromScalarNode(doc, yaml_document_get_node(doc, pair->key)),
        ValueFromNode(doc, yaml_document_get_node(doc, pair->value)));
    pair++;
  }
  return result;
}


static Handle<Value>
ValueFromNode(yaml_document_t *doc, yaml_node_t *node)
{
  switch (node->type) {
    case YAML_SCALAR_NODE:   return ValueFromScalarNode(doc, node);
    case YAML_SEQUENCE_NODE: return ValueFromSequenceNode(doc, node);
    case YAML_MAPPING_NODE:  return ValueFromMappingNode(doc, node);
    default: return Null();
  }
}


static Handle<Value>
Load(const Arguments &args)
{
  HandleScope scope;
  Local<Array> documents = Array::New();

  /* Check arguments */
  if (args.Length() != 1)
    return ThrowException(Exception::Error(String::New("One arguments was expected.")));
  if (!args[0]->IsString()) 
    return ThrowException(Exception::TypeError(String::New("Input must be a string.")));

  /* Initialize parser. */
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
    return ThrowException(Exception::Error(String::New("YAML parser initialization failed.")));
  /* FIXME: Detect endianness? */
  yaml_parser_set_encoding(&parser, YAML_UTF16LE_ENCODING);
  String::Value input(args[0]);
  yaml_parser_set_input_string(&parser,
      (const unsigned char *)*input, input.length() * sizeof(uint16_t));

  /* Iterate documents. */
  int success;
  yaml_document_t document;
  for (uint32_t i = 0; (success = yaml_parser_load(&parser, &document)); i++) {
    yaml_node_t *root = yaml_document_get_root_node(&document);
    if (root)
      documents->Set(i, ValueFromNode(&document, root));
    yaml_document_delete(&document);
    if (!root)
      break;
  }
  /* Success should be 1 if we break'd as we were supposed to. */
  /* FIXME: More descriptive error message. */
  if (!success)
    return ThrowException(Exception::Error(String::New("YAML parser had an error.")));

  /* Clean up the parser. */
  yaml_parser_delete(&parser);

  return scope.Close(documents);
}


static void
Initialize(Handle<Object> target)
{
  HandleScope scope;
  Local<FunctionTemplate> load_template = FunctionTemplate::New(Load);
  target->Set(String::NewSymbol("load"), load_template->GetFunction());
}


}


extern "C" void
init(Handle<Object> target)
{
  yaml::Initialize(target);
}
