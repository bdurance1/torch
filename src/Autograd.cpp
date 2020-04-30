#include <iostream>

#define LANTERN_BUILD

#include "lantern/lantern.h"

#include <torch/torch.h>

#include "utils.hpp"
#include "Function.h"
#include <thread>

void lantern_autograd_set_grad_mode(bool enabled)
{
    torch::autograd::GradMode::set_enabled(enabled);
}

void *lantern_Tensor_grad(void *self)
{
    auto out = reinterpret_cast<LanternObject<torch::Tensor> *>(self)->get().grad();
    return (void *)new LanternObject<torch::Tensor>(out);
}

bool lantern_Tensor_requires_grad(void *self)
{
    return reinterpret_cast<LanternObject<torch::Tensor> *>(self)->get().requires_grad();
}

unsigned int lantern_Tensor_register_hook(void *self, void *hook)
{
    auto h = reinterpret_cast<LanternObject<std::function<torch::Tensor(torch::Tensor)>> *>(hook)->get();
    auto x = reinterpret_cast<LanternObject<torch::Tensor> *>(self)->get();
    return x.register_hook(h);
}

// Creating the hook in the right format to be passed to .register_hook
// It takes a pointer a function that in turn will take a pointer to a
// torch tensor and a function to apply over it.
// fun must return a pointer To a lantern object of type tensor.
void *lantern_new_hook(void *(*fun)(void *, void *), void *custom)
{
    auto out = [fun, custom](torch::Tensor grad) {
        auto out = (*fun)((void *)new LanternObject<torch::Tensor>(grad), custom);
        auto ten = reinterpret_cast<LanternObject<torch::Tensor> *>(out)->get();
        return ten;
    };
    return (void *)new LanternObject<std::function<torch::Tensor(torch::Tensor)>>(out);
}

void lantern_Tensor_remove_hook(void *self, unsigned int pos)
{
    reinterpret_cast<LanternObject<torch::Tensor> *>(self)->get().remove_hook(pos);
}

void *lantern_variable_list_new()
{
    auto out = new LanternObject<torch::autograd::variable_list>();
    return (void *)out;
}

void lantern_variable_list_push_back(void *self, void *x)
{
    auto t = reinterpret_cast<LanternObject<torch::Tensor> *>(x)->get();
    reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(self)->get().push_back(t);
}

void *lantern_variable_list_get(void *self, int64_t i)
{
    auto s = reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(self)->get();
    torch::Tensor out = s[i];
    return (void *)new LanternObject<torch::Tensor>(out);
}

int64_t lantern_variable_list_size(void *self)
{
    auto s = reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(self)->get();
    return s.size();
}

void *lantern_Function_lambda(void *(*fun)(void *, void *, void *), void *custom)
{
    auto out = [fun, custom](torch::autograd::LanternAutogradContext *ctx, torch::autograd::variable_list inputs) {
        auto out = (*fun)(custom, (void *)ctx, (void *)new LanternObject<torch::autograd::variable_list>(inputs));
        auto vl = reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(out)->get();
        return vl;
    };
    return (void *)new LanternObject<std::function<torch::autograd::variable_list(torch::autograd::LanternAutogradContext *, torch::autograd::variable_list)>>(out);
}

void *lantern_Function_apply(void *inputs, void *forward, void *backward)
{
    auto out = torch::autograd::LanternFunction::apply(
        reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(inputs)->get(),
        reinterpret_cast<LanternObject<std::function<torch::autograd::variable_list(torch::autograd::LanternAutogradContext *, torch::autograd::variable_list)>> *>(forward)->get(),
        reinterpret_cast<LanternObject<std::function<torch::autograd::variable_list(torch::autograd::LanternAutogradContext *, torch::autograd::variable_list)>> *>(backward)->get());

    return (void *)new LanternObject<torch::autograd::variable_list>(out);
}

void lantern_AutogradContext_save_for_backward(void *self, void *vars)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    ctx->save_for_backward(reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(vars)->get());
}

void *lantern_AutogradContext_get_saved_variables(void *self)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    return (void *)new LanternObject<torch::autograd::variable_list>(ctx->get_saved_variables());
}

void lantern_AutogradContext_set_arguments(void *self, void *names, void *needs_grad)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    ctx->set_arguments(
        *reinterpret_cast<std::vector<std::string> *>(names),
        *reinterpret_cast<std::vector<bool> *>(needs_grad));
}

void *lantern_AutogradContext_get_argument_names(void *self)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    return (void *)new std::vector<std::string>(ctx->get_argument_names());
}

void *lantern_AutogradContext_get_argument_needs_grad(void *self)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    return (void *)new std::vector<bool>(ctx->get_argument_needs_grad());
}

void lantern_AutogradContext_set_saved_variables_names(void *self, void *names)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    ctx->set_saved_variables_names(*reinterpret_cast<std::vector<std::string> *>(names));
}

void *lantern_AutogradContext_get_saved_variables_names(void *self)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    return (void *)new std::vector<std::string>(ctx->get_saved_variables_names());
}

void lantern_AutogradContext_mark_dirty(void *self, void *inputs)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    auto vars = reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(inputs)->get();
    ctx->mark_dirty(vars);
}

void lantern_AutogradContext_mark_non_differentiable(void *self, void *outputs)
{
    auto ctx = reinterpret_cast<torch::autograd::LanternAutogradContext *>(self);
    auto vars = reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(outputs)->get();
    ctx->mark_non_differentiable(vars);
}

void lantern_autograd_backward(void *tensors, void *grad_tensors, void *retain_graph, bool create_graph)
{
    torch::autograd::backward(((LanternObject<torch::autograd::variable_list> *)tensors)->get(), ((LanternObject<torch::autograd::variable_list> *)grad_tensors)->get(), ((LanternObject<bool> *)retain_graph)->get(), ((LanternObject<bool> *)create_graph)->get());
}

void *lantern_autograd_grad(void *outputs, void *inputs, void *grad_outputs,
                            void *retain_graph, bool create_graph, bool allow_unused)
{
    auto out = torch::autograd::grad(
        reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(outputs)->get(),
        reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(inputs)->get(),
        reinterpret_cast<LanternObject<torch::autograd::variable_list> *>(grad_outputs)->get(),
        reinterpret_cast<LanternObject<c10::optional<bool>> *>(retain_graph)->get(),
        create_graph,
        allow_unused);
    return (void *)new LanternObject<torch::autograd::variable_list>(out);
}

void *lantern_Tensor_grad_fn(void *self)
{
    auto t = reinterpret_cast<LanternObject<torch::Tensor> *>(self)->get();
    auto f = t.grad_fn().get();
    return (void *)f;
}
