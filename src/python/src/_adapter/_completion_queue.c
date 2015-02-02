/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "_adapter/_completion_queue.h"

#include <Python.h>
#include <grpc/grpc.h>
#include <grpc/support/alloc.h>

#include "_adapter/_call.h"

static PyObject *status_class;
static PyObject *service_acceptance_class;
static PyObject *event_class;

static PyObject *ok_status_code;
static PyObject *cancelled_status_code;
static PyObject *unknown_status_code;
static PyObject *invalid_argument_status_code;
static PyObject *expired_status_code;
static PyObject *not_found_status_code;
static PyObject *already_exists_status_code;
static PyObject *permission_denied_status_code;
static PyObject *unauthenticated_status_code;
static PyObject *resource_exhausted_status_code;
static PyObject *failed_precondition_status_code;
static PyObject *aborted_status_code;
static PyObject *out_of_range_status_code;
static PyObject *unimplemented_status_code;
static PyObject *internal_error_status_code;
static PyObject *unavailable_status_code;
static PyObject *data_loss_status_code;

static PyObject *stop_event_kind;
static PyObject *write_event_kind;
static PyObject *complete_event_kind;
static PyObject *service_event_kind;
static PyObject *read_event_kind;
static PyObject *metadata_event_kind;
static PyObject *finish_event_kind;

static PyObject *pygrpc_as_py_time(gpr_timespec *timespec) {
  return Py_BuildValue("f",
                       timespec->tv_sec + ((double)timespec->tv_nsec) / 1.0E9);
}

static PyObject *pygrpc_status_code(grpc_status_code c_status_code) {
  switch (c_status_code) {
    case GRPC_STATUS_OK:
      return ok_status_code;
    case GRPC_STATUS_CANCELLED:
      return cancelled_status_code;
    case GRPC_STATUS_UNKNOWN:
      return unknown_status_code;
    case GRPC_STATUS_INVALID_ARGUMENT:
      return invalid_argument_status_code;
    case GRPC_STATUS_DEADLINE_EXCEEDED:
      return expired_status_code;
    case GRPC_STATUS_NOT_FOUND:
      return not_found_status_code;
    case GRPC_STATUS_ALREADY_EXISTS:
      return already_exists_status_code;
    case GRPC_STATUS_PERMISSION_DENIED:
      return permission_denied_status_code;
    case GRPC_STATUS_UNAUTHENTICATED:
      return unauthenticated_status_code;
    case GRPC_STATUS_RESOURCE_EXHAUSTED:
      return resource_exhausted_status_code;
    case GRPC_STATUS_FAILED_PRECONDITION:
      return failed_precondition_status_code;
    case GRPC_STATUS_ABORTED:
      return aborted_status_code;
    case GRPC_STATUS_OUT_OF_RANGE:
      return out_of_range_status_code;
    case GRPC_STATUS_UNIMPLEMENTED:
      return unimplemented_status_code;
    case GRPC_STATUS_INTERNAL:
      return internal_error_status_code;
    case GRPC_STATUS_UNAVAILABLE:
      return unavailable_status_code;
    case GRPC_STATUS_DATA_LOSS:
      return data_loss_status_code;
    default:
      return NULL;
  }
}

static PyObject *pygrpc_stop_event_args(grpc_event *c_event) {
  return Py_BuildValue("(OOOOOOO)", stop_event_kind, Py_None, Py_None, Py_None,
                       Py_None, Py_None, Py_None);
}

static PyObject *pygrpc_write_event_args(grpc_event *c_event) {
  PyObject *write_accepted =
      c_event->data.write_accepted == GRPC_OP_OK ? Py_True : Py_False;
  return Py_BuildValue("(OOOOOOO)", write_event_kind, (PyObject *)c_event->tag,
                       write_accepted, Py_None, Py_None, Py_None, Py_None);
}

static PyObject *pygrpc_complete_event_args(grpc_event *c_event) {
  PyObject *complete_accepted =
      c_event->data.finish_accepted == GRPC_OP_OK ? Py_True : Py_False;
  return Py_BuildValue("(OOOOOOO)", complete_event_kind,
                       (PyObject *)c_event->tag, Py_None, complete_accepted,
                       Py_None, Py_None, Py_None);
}

static PyObject *pygrpc_service_event_args(grpc_event *c_event) {
  if (c_event->data.server_rpc_new.method == NULL) {
    return Py_BuildValue("(OOOOOOO)", service_event_kind, c_event->tag,
                         Py_None, Py_None, Py_None, Py_None, Py_None);
  } else {
    PyObject *method = PyBytes_FromString(c_event->data.server_rpc_new.method);
    PyObject *host = PyBytes_FromString(c_event->data.server_rpc_new.host);
    PyObject *service_deadline =
        pygrpc_as_py_time(&c_event->data.server_rpc_new.deadline);

    Call *call;
    PyObject *service_acceptance_args;
    PyObject *service_acceptance;
    PyObject *event_args;

    call = PyObject_New(Call, &pygrpc_CallType);
    call->c_call = c_event->call;

    service_acceptance_args =
        Py_BuildValue("(OOOO)", call, method, host, service_deadline);
    Py_DECREF(call);
    Py_DECREF(method);
    Py_DECREF(host);
    Py_DECREF(service_deadline);

    service_acceptance =
        PyObject_CallObject(service_acceptance_class, service_acceptance_args);
    Py_DECREF(service_acceptance_args);

    event_args = Py_BuildValue("(OOOOOOO)", service_event_kind,
                               (PyObject *)c_event->tag, Py_None, Py_None,
                               service_acceptance, Py_None, Py_None);
    Py_DECREF(service_acceptance);
    return event_args;
  }
}

static PyObject *pygrpc_read_event_args(grpc_event *c_event) {
  if (c_event->data.read == NULL) {
    return Py_BuildValue("(OOOOOOO)", read_event_kind,
                         (PyObject *)c_event->tag, Py_None, Py_None, Py_None,
                         Py_None, Py_None);
  } else {
    size_t length;
    size_t offset;
    grpc_byte_buffer_reader *reader;
    gpr_slice slice;
    char *c_bytes;
    PyObject *bytes;
    PyObject *event_args;

    length = grpc_byte_buffer_length(c_event->data.read);
    reader = grpc_byte_buffer_reader_create(c_event->data.read);
    c_bytes = gpr_malloc(length);
    offset = 0;
    while (grpc_byte_buffer_reader_next(reader, &slice)) {
      memcpy(c_bytes + offset, GPR_SLICE_START_PTR(slice),
             GPR_SLICE_LENGTH(slice));
      offset += GPR_SLICE_LENGTH(slice);
    }
    grpc_byte_buffer_reader_destroy(reader);
    bytes = PyBytes_FromStringAndSize(c_bytes, length);
    gpr_free(c_bytes);
    event_args =
        Py_BuildValue("(OOOOOOO)", read_event_kind, (PyObject *)c_event->tag,
                      Py_None, Py_None, Py_None, bytes, Py_None);
    Py_DECREF(bytes);
    return event_args;
  }
}

static PyObject *pygrpc_metadata_event_args(grpc_event *c_event) {
  /* TODO(nathaniel): Actual transmission of metadata. */
  return Py_BuildValue("(OOOOOOO)", metadata_event_kind,
                       (PyObject *)c_event->tag, Py_None, Py_None, Py_None,
                       Py_None, Py_None);
}

static PyObject *pygrpc_finished_event_args(grpc_event *c_event) {
  PyObject *code;
  PyObject *details;
  PyObject *status_args;
  PyObject *status;
  PyObject *event_args;

  code = pygrpc_status_code(c_event->data.finished.status);
  if (code == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Unrecognized status code!");
    return NULL;
  }
  if (c_event->data.finished.details == NULL) {
    details = PyBytes_FromString("");
  } else {
    details = PyBytes_FromString(c_event->data.finished.details);
  }
  status_args = Py_BuildValue("(OO)", code, details);
  Py_DECREF(details);
  status = PyObject_CallObject(status_class, status_args);
  Py_DECREF(status_args);
  event_args =
      Py_BuildValue("(OOOOOOO)", finish_event_kind, (PyObject *)c_event->tag,
                    Py_None, Py_None, Py_None, Py_None, status);
  Py_DECREF(status);
  return event_args;
}

static int pygrpc_completion_queue_init(CompletionQueue *self, PyObject *args,
                                        PyObject *kwds) {
  self->c_completion_queue = grpc_completion_queue_create();
  return 0;
}

static void pygrpc_completion_queue_dealloc(CompletionQueue *self) {
  grpc_completion_queue_destroy(self->c_completion_queue);
  self->ob_type->tp_free((PyObject *)self);
}

static PyObject *pygrpc_completion_queue_get(CompletionQueue *self,
                                             PyObject *args) {
  PyObject *deadline;
  double double_deadline;
  gpr_timespec deadline_timespec;
  grpc_event *c_event;

  PyObject *event_args;
  PyObject *event;

  if (!(PyArg_ParseTuple(args, "O", &deadline))) {
    return NULL;
  }

  if (deadline == Py_None) {
    deadline_timespec = gpr_inf_future;
  } else {
    double_deadline = PyFloat_AsDouble(deadline);
    deadline_timespec = gpr_time_from_nanos((long)(double_deadline * 1.0E9));
  }

  /* TODO(nathaniel): Suppress clang-format in this block and remove the
     unnecessary and unPythonic semicolons trailing the _ALLOW_THREADS macros.
     (Right now clang-format only understands //-demarcated suppressions.) */
  Py_BEGIN_ALLOW_THREADS;
  c_event =
      grpc_completion_queue_next(self->c_completion_queue, deadline_timespec);
  Py_END_ALLOW_THREADS;

  if (c_event == NULL) {
    Py_RETURN_NONE;
  }

  switch (c_event->type) {
    case GRPC_QUEUE_SHUTDOWN:
      event_args = pygrpc_stop_event_args(c_event);
      break;
    case GRPC_WRITE_ACCEPTED:
      event_args = pygrpc_write_event_args(c_event);
      break;
    case GRPC_FINISH_ACCEPTED:
      event_args = pygrpc_complete_event_args(c_event);
      break;
    case GRPC_SERVER_RPC_NEW:
      event_args = pygrpc_service_event_args(c_event);
      break;
    case GRPC_READ:
      event_args = pygrpc_read_event_args(c_event);
      break;
    case GRPC_CLIENT_METADATA_READ:
      event_args = pygrpc_metadata_event_args(c_event);
      break;
    case GRPC_FINISHED:
      event_args = pygrpc_finished_event_args(c_event);
      break;
    default:
      PyErr_SetString(PyExc_Exception, "Unrecognized event type!");
      return NULL;
  }

  if (event_args == NULL) {
    return NULL;
  }

  event = PyObject_CallObject(event_class, event_args);

  Py_DECREF(event_args);
  Py_XDECREF((PyObject *)c_event->tag);
  grpc_event_finish(c_event);

  return event;
}

static PyObject *pygrpc_completion_queue_stop(CompletionQueue *self) {
  grpc_completion_queue_shutdown(self->c_completion_queue);

  Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    {"get", (PyCFunction)pygrpc_completion_queue_get, METH_VARARGS,
     "Get the next event."},
    {"stop", (PyCFunction)pygrpc_completion_queue_stop, METH_NOARGS,
     "Stop this completion queue."},
    {NULL}};

PyTypeObject pygrpc_CompletionQueueType = {
    PyObject_HEAD_INIT(NULL)0,                   /*ob_size*/
    "_gprc.CompletionQueue",                     /*tp_name*/
    sizeof(CompletionQueue),                     /*tp_basicsize*/
    0,                                           /*tp_itemsize*/
    (destructor)pygrpc_completion_queue_dealloc, /*tp_dealloc*/
    0,                                           /*tp_print*/
    0,                                           /*tp_getattr*/
    0,                                           /*tp_setattr*/
    0,                                           /*tp_compare*/
    0,                                           /*tp_repr*/
    0,                                           /*tp_as_number*/
    0,                                           /*tp_as_sequence*/
    0,                                           /*tp_as_mapping*/
    0,                                           /*tp_hash */
    0,                                           /*tp_call*/
    0,                                           /*tp_str*/
    0,                                           /*tp_getattro*/
    0,                                           /*tp_setattro*/
    0,                                           /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                          /*tp_flags*/
    "Wrapping of grpc_completion_queue.",        /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    methods,                                     /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    (initproc)pygrpc_completion_queue_init,      /* tp_init */
};

static int pygrpc_get_status_codes(PyObject *datatypes_module) {
  PyObject *code_class = PyObject_GetAttrString(datatypes_module, "Code");
  if (code_class == NULL) {
    return -1;
  }
  ok_status_code = PyObject_GetAttrString(code_class, "OK");
  if (ok_status_code == NULL) {
    return -1;
  }
  cancelled_status_code = PyObject_GetAttrString(code_class, "CANCELLED");
  if (cancelled_status_code == NULL) {
    return -1;
  }
  unknown_status_code = PyObject_GetAttrString(code_class, "UNKNOWN");
  if (unknown_status_code == NULL) {
    return -1;
  }
  invalid_argument_status_code =
      PyObject_GetAttrString(code_class, "INVALID_ARGUMENT");
  if (invalid_argument_status_code == NULL) {
    return -1;
  }
  expired_status_code = PyObject_GetAttrString(code_class, "EXPIRED");
  if (expired_status_code == NULL) {
    return -1;
  }
  not_found_status_code = PyObject_GetAttrString(code_class, "NOT_FOUND");
  if (not_found_status_code == NULL) {
    return -1;
  }
  already_exists_status_code =
      PyObject_GetAttrString(code_class, "ALREADY_EXISTS");
  if (already_exists_status_code == NULL) {
    return -1;
  }
  permission_denied_status_code =
      PyObject_GetAttrString(code_class, "PERMISSION_DENIED");
  if (permission_denied_status_code == NULL) {
    return -1;
  }
  unauthenticated_status_code =
      PyObject_GetAttrString(code_class, "UNAUTHENTICATED");
  if (unauthenticated_status_code == NULL) {
    return -1;
  }
  resource_exhausted_status_code =
      PyObject_GetAttrString(code_class, "RESOURCE_EXHAUSTED");
  if (resource_exhausted_status_code == NULL) {
    return -1;
  }
  failed_precondition_status_code =
      PyObject_GetAttrString(code_class, "FAILED_PRECONDITION");
  if (failed_precondition_status_code == NULL) {
    return -1;
  }
  aborted_status_code = PyObject_GetAttrString(code_class, "ABORTED");
  if (aborted_status_code == NULL) {
    return -1;
  }
  out_of_range_status_code = PyObject_GetAttrString(code_class, "OUT_OF_RANGE");
  if (out_of_range_status_code == NULL) {
    return -1;
  }
  unimplemented_status_code =
      PyObject_GetAttrString(code_class, "UNIMPLEMENTED");
  if (unimplemented_status_code == NULL) {
    return -1;
  }
  internal_error_status_code =
      PyObject_GetAttrString(code_class, "INTERNAL_ERROR");
  if (internal_error_status_code == NULL) {
    return -1;
  }
  unavailable_status_code = PyObject_GetAttrString(code_class, "UNAVAILABLE");
  if (unavailable_status_code == NULL) {
    return -1;
  }
  data_loss_status_code = PyObject_GetAttrString(code_class, "DATA_LOSS");
  if (data_loss_status_code == NULL) {
    return -1;
  }
  Py_DECREF(code_class);
  return 0;
}

static int pygrpc_get_event_kinds(PyObject *event_class) {
  PyObject *kind_class = PyObject_GetAttrString(event_class, "Kind");
  if (kind_class == NULL) {
    return -1;
  }
  stop_event_kind = PyObject_GetAttrString(kind_class, "STOP");
  if (stop_event_kind == NULL) {
    return -1;
  }
  write_event_kind = PyObject_GetAttrString(kind_class, "WRITE_ACCEPTED");
  if (write_event_kind == NULL) {
    return -1;
  }
  complete_event_kind = PyObject_GetAttrString(kind_class, "COMPLETE_ACCEPTED");
  if (complete_event_kind == NULL) {
    return -1;
  }
  service_event_kind = PyObject_GetAttrString(kind_class, "SERVICE_ACCEPTED");
  if (service_event_kind == NULL) {
    return -1;
  }
  read_event_kind = PyObject_GetAttrString(kind_class, "READ_ACCEPTED");
  if (read_event_kind == NULL) {
    return -1;
  }
  metadata_event_kind = PyObject_GetAttrString(kind_class, "METADATA_ACCEPTED");
  if (metadata_event_kind == NULL) {
    return -1;
  }
  finish_event_kind = PyObject_GetAttrString(kind_class, "FINISH");
  if (finish_event_kind == NULL) {
    return -1;
  }
  Py_DECREF(kind_class);
  return 0;
}

int pygrpc_add_completion_queue(PyObject *module) {
  char *datatypes_module_path = "_adapter._datatypes";
  PyObject *datatypes_module = PyImport_ImportModule(datatypes_module_path);
  if (datatypes_module == NULL) {
    PyErr_SetString(PyExc_ImportError, datatypes_module_path);
    return -1;
  }
  status_class = PyObject_GetAttrString(datatypes_module, "Status");
  service_acceptance_class =
      PyObject_GetAttrString(datatypes_module, "ServiceAcceptance");
  event_class = PyObject_GetAttrString(datatypes_module, "Event");
  if (status_class == NULL || service_acceptance_class == NULL ||
      event_class == NULL) {
    PyErr_SetString(PyExc_ImportError, "Missing classes in _datatypes module!");
    return -1;
  }
  if (pygrpc_get_status_codes(datatypes_module) == -1) {
    PyErr_SetString(PyExc_ImportError, "Status codes import broken!");
    return -1;
  }
  if (pygrpc_get_event_kinds(event_class) == -1) {
    PyErr_SetString(PyExc_ImportError, "Event kinds import broken!");
    return -1;
  }
  Py_DECREF(datatypes_module);

  pygrpc_CompletionQueueType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&pygrpc_CompletionQueueType) < 0) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Error defining pygrpc_CompletionQueueType!");
    return -1;
  }
  if (PyModule_AddObject(module, "CompletionQueue",
                         (PyObject *)&pygrpc_CompletionQueueType) == -1) {
    PyErr_SetString(PyExc_ImportError,
                    "Couldn't add CompletionQueue type to module!");
    return -1;
  }
  return 0;
}