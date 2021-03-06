// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: health.proto
#region Designer generated code

using System;
using System.Threading;
using System.Threading.Tasks;
using Grpc.Core;

namespace Grpc.Health.V1Alpha {
  public static class Health
  {
    static readonly string __ServiceName = "grpc.health.v1alpha.Health";

    static readonly Marshaller<global::Grpc.Health.V1Alpha.HealthCheckRequest> __Marshaller_HealthCheckRequest = Marshallers.Create((arg) => arg.ToByteArray(), global::Grpc.Health.V1Alpha.HealthCheckRequest.ParseFrom);
    static readonly Marshaller<global::Grpc.Health.V1Alpha.HealthCheckResponse> __Marshaller_HealthCheckResponse = Marshallers.Create((arg) => arg.ToByteArray(), global::Grpc.Health.V1Alpha.HealthCheckResponse.ParseFrom);

    static readonly Method<global::Grpc.Health.V1Alpha.HealthCheckRequest, global::Grpc.Health.V1Alpha.HealthCheckResponse> __Method_Check = new Method<global::Grpc.Health.V1Alpha.HealthCheckRequest, global::Grpc.Health.V1Alpha.HealthCheckResponse>(
        MethodType.Unary,
        "Check",
        __Marshaller_HealthCheckRequest,
        __Marshaller_HealthCheckResponse);

    // client interface
    public interface IHealthClient
    {
      global::Grpc.Health.V1Alpha.HealthCheckResponse Check(global::Grpc.Health.V1Alpha.HealthCheckRequest request, Metadata headers = null, CancellationToken cancellationToken = default(CancellationToken));
      AsyncUnaryCall<global::Grpc.Health.V1Alpha.HealthCheckResponse> CheckAsync(global::Grpc.Health.V1Alpha.HealthCheckRequest request, Metadata headers = null, CancellationToken cancellationToken = default(CancellationToken));
    }

    // server-side interface
    public interface IHealth
    {
      Task<global::Grpc.Health.V1Alpha.HealthCheckResponse> Check(global::Grpc.Health.V1Alpha.HealthCheckRequest request, ServerCallContext context);
    }

    // client stub
    public class HealthClient : ClientBase, IHealthClient
    {
      public HealthClient(Channel channel) : base(channel)
      {
      }
      public global::Grpc.Health.V1Alpha.HealthCheckResponse Check(global::Grpc.Health.V1Alpha.HealthCheckRequest request, Metadata headers = null, CancellationToken cancellationToken = default(CancellationToken))
      {
        var call = CreateCall(__ServiceName, __Method_Check, headers);
        return Calls.BlockingUnaryCall(call, request, cancellationToken);
      }
      public AsyncUnaryCall<global::Grpc.Health.V1Alpha.HealthCheckResponse> CheckAsync(global::Grpc.Health.V1Alpha.HealthCheckRequest request, Metadata headers = null, CancellationToken cancellationToken = default(CancellationToken))
      {
        var call = CreateCall(__ServiceName, __Method_Check, headers);
        return Calls.AsyncUnaryCall(call, request, cancellationToken);
      }
    }

    // creates service definition that can be registered with a server
    public static ServerServiceDefinition BindService(IHealth serviceImpl)
    {
      return ServerServiceDefinition.CreateBuilder(__ServiceName)
          .AddMethod(__Method_Check, serviceImpl.Check).Build();
    }

    // creates a new client
    public static HealthClient NewClient(Channel channel)
    {
      return new HealthClient(channel);
    }

  }
}
#endregion
