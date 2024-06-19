/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    faCodeBranch,
    faCubes,
    faMicrochip,
    faTrashCan
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { useMemo } from "react";
import { Card, CardProps } from "react-daisyui";

export type CommitCardProps = CardProps & {
    section: ISection;
    commit: Commit;
    onActivate?: (section: ISection, commit: Commit) => void;
    onDeleteRequested?: (section: ISection) => void;
};

export default (props: CommitCardProps) => {
    const { branch, repository, architecture } = useMemo(
        () => props.section,
        [props.section]
    );

    return (
        <Card
            onClick={() => props.onActivate?.(props.section, props.commit)}
            {...props}
            className="h-30 bg-white"
        >
            <div className="w-full flex justify-end">
                <FontAwesomeIcon
                    onClick={(e) => {
                        e.stopPropagation();
                        if (props.onDeleteRequested)
                            return props.onDeleteRequested(props.section);
                    }}
                    className="px-1"
                    icon={faTrashCan}
                />
            </div>
            <Card.Body className="font-bold">
                {props.commit.size} package
                {props.commit.size == 1 ? "" : "s"}
            </Card.Body>
            <span className="space-x-4">
                <FontAwesomeIcon className="px-1" icon={faCodeBranch} />
                {branch}
                <FontAwesomeIcon className="px-1" icon={faCubes} />
                {repository}
                <FontAwesomeIcon className="px-1" icon={faMicrochip} />
                {architecture}
            </span>
        </Card>
    );
};
