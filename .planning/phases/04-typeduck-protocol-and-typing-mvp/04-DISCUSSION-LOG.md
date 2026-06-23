# Phase 4: TypeDuck Protocol and Typing MVP - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-06-23T18:02:17Z
**Phase:** 4-TypeDuck Protocol and Typing MVP
**Areas discussed:** Protocol shape and versioning, Candidate and dictionary data mapping, Typing MVP behavior, Responsiveness and failure policy

---

## Protocol Shape and Versioning

| Option | Description | Selected |
|--------|-------------|----------|
| Keep transport mechanics, replace/extend semantics carefully | Preserve TSF named pipe, launcher backend stdio, framed protobuf mechanics; treat existing Moqi schema only as a compatibility bridge. | yes |
| Replace the Moqi protobuf wholesale now | Cleaner long term, but higher risk because TSF, launcher, backend adapter, fixtures, and tests all move at once. | |
| Keep Moqi protocol semantics and only reinterpret comments | Fastest, but fails the TypeDuck product-truth boundary. | |

**User's choice:** Accepted after clarification.
**Notes:** User asked what "existing Moqi protobuf" and "current transport" mean. Clarified that `proto/moqi.proto` contains the existing schema (`ClientRequest`, `ServerResponse`, `Method`, `KeyEvent`, `CandidateEntry`, etc.), while transport means TSF-to-launcher named pipe, launcher-to-backend stdin/stdout, and 32-bit length-prefixed protobuf frames. User accepted keeping transport mechanics without keeping Moqi product semantics as truth.

---

## Candidate and Dictionary Data Mapping

| Option | Description | Selected |
|--------|-------------|----------|
| Parse lookup-filter payload into structured TypeDuck fields in Phase 4 | Candidate id/text/input code/Jyutping/comments/translations/dictionary rows become first-class protocol data. | |
| Leave parsing to Phase 5 and preserve raw comment payload in the current comment display position | Phase 4 proves raw payload survivability and typing; Phase 5 owns dictionary parsing/UI. | yes |
| Hybrid: structured candidate basics, raw dictionary blob retained | Useful bridge, but risks premature UI-model design. | |

**User's choice:** Leave parsing to Phase 5.
**Notes:** User stated that parsing should be treated as part of UI and that a raw payload at the current comment display position is sufficient for Phase 4.

---

## Typing MVP Behavior

| Option | Description | Selected |
|--------|-------------|----------|
| Core Cantonese typing MVP | Composition, candidate list, paging, numeric selection, commit, candidate count, and stable reset. | yes |
| Core MVP plus frontend reverse lookup/Cangjie logic | Initially proposed as a separate frontend concern, but corrected by the user. | |
| Only prove protocol fixtures and defer real TSF typing | Too thin for the phase goal. | |

**User's choice:** Core frontend typing MVP; reverse lookup and Cangjie are not special frontend features.
**Notes:** User clarified that reverse lookup is handled through librime/backend and has nothing to do with the frontend beyond keypress capture and forwarding. Therefore the original options 1 and 2 are equivalent from the frontend's perspective.

---

## Responsiveness and Failure Policy

| Option | Description | Selected |
|--------|-------------|----------|
| Add bounded frame/timeout/error behavior | Frame size caps, malformed-frame rejection, bounded buffering, backend health/error/degraded handling, and clean reset. | yes |
| Restart backend on timeout and keep current client behavior | Minimal churn, but current code has unclear timeout/init failure responses. | |
| Move engine in-process to avoid launcher latency | Not aligned with prior decisions unless the adapter path concretely blocks. | |

**User's choice:** Accepted bounded hardening after asking whether Moqi is buggy in these areas.
**Notes:** Clarified that Moqi is not necessarily unusable, but the scaffold is brittle for TypeDuck guarantees. Evidence includes no max frame cap, timeout paths that restart the backend without a clear client error response, backend init failure FIXME, synchronous RPC on the typing path, and TODO-noted mixed Chinese/English or Shift switching issues.

---

## the agent's Discretion

- Planner may choose the exact protocol representation: new protobuf fields/messages, wrapper helpers, or compatibility adapter layer.
- Planner may decide exact test split, provided raw comment preservation, frame bounds, malformed payloads, timeout/restart behavior, and TSF typing smoke are covered.

## Deferred Ideas

- Lookup-filter dictionary parsing and structured dictionary UI fields are deferred to Phase 5.
- Native candidate/dictionary visual parity, movement-triggered reveal, settings, and About surfaces are deferred to Phase 5.
- Broad scaffold cleanup and banned-surface audits are deferred to Phase 6 unless required for Phase 4 typing correctness.
